#include "master.h"
#include "steam_data.h"
#include "player.h"
#include "numstr.h"
#include "menu.h"

#ifdef CMAKE_BUILD
#include "steam_api.h"
#endif

using namespace pyrodactyl;

// Steam UserStats interface
static ISteamUserStats *m_pSteamUserStats;

//Let's see how many people read any of Rutskarn's text
int news_count, tutorial_count;

void SteamUserInit()
{
	m_pSteamUserStats = nullptr;
	m_pSteamUserStats = SteamUserStats();

	if (m_pSteamUserStats != nullptr)
	{
		m_pSteamUserStats->RequestCurrentStats();
		MenuLoadLeaderboards();
	}

	news_count = 0;
	tutorial_count = 0;
}

ISteamUserStats* PlayerSteamStats()
{
	return m_pSteamUserStats;
}

bool SteamUserValid()
{
	return (m_pSteamUserStats != nullptr && EnvAchievementsEnabled());
}

//Achievements
void GiveAchievement(const char* ach_id)
{
	if (SteamUserValid())
	{
		m_pSteamUserStats->SetAchievement(ach_id);
		m_pSteamUserStats->StoreStats();
	}
}

void SteamIndicateAchievementProgress(const char* ach_id, const int cur, const int max)
{
	if (SteamUserValid())
	{
		m_pSteamUserStats->IndicateAchievementProgress(ach_id, cur, max);
		m_pSteamUserStats->StoreStats();
	}
}

//Stat tracking
void SteamSetStat(const char* stat_id, const int value)
{
	if (SteamUserValid())
		m_pSteamUserStats->SetStat(stat_id, value);
}

void SteamSetStat(const char* stat_id, const float value)
{
	if (SteamUserValid())
		m_pSteamUserStats->SetStat(stat_id, value);
}

void SteamUpdateAvgRate(const char* stat_id, const float value, const float session_length)
{
	if (SteamUserValid())
		m_pSteamUserStats->UpdateAvgRateStat(stat_id, value, session_length);
}

void SteamSync()
{
	if (SteamUserValid())
		m_pSteamUserStats->StoreStats();
}

template <typename T, typename U>
bool SteamCheckAch(const T val, const char* ach_id, const U quantity)
{
	bool ach_unlocked = false;
	if (SteamUserValid())
	{
		m_pSteamUserStats->GetAchievement(ach_id, &ach_unlocked);
		if (!ach_unlocked && val >= quantity)
		{
			m_pSteamUserStats->SetAchievement(ach_id);
			ach_unlocked = true;
		}
	}

	return ach_unlocked;
}

const char* SteamDisplayName()
{
	if (SteamFriends() != nullptr)
		return SteamFriends()->GetPersonaName();

	return "Player";
}

//Player stats are synced to steam
//The achievement checks made here are for "within a single game" only
void PlayerStats::TriviaCheckAch(PlayerTrivia index)
{
	int old_value = 0;
	std::string stat_name;
	bool sync = false;

	switch (index)
	{
	case TRIVIA_DAMAGE_TAKEN:       stat_name = "DamageTaken"; break;
	case TRIVIA_DAMAGE_DEALT:       stat_name = "DamageDealt"; break;
	case TRIVIA_XP_GATHERED:
		stat_name = "MoneyGathered";
		sync |= SteamCheckAch(_trivia[index], "ach_money_10k", 10000);
		sync |= SteamCheckAch(_trivia[index], "ach_money_100k", 100000);
		break;
	case TRIVIA_BULLETS_FIRED:      stat_name = "BulletsFired"; break;
	case TRIVIA_KILLS:
		stat_name = "EnemiesKilled";
		sync |= SteamCheckAch(_trivia[index], "ach_kill_500", 500);
		sync |= SteamCheckAch(_trivia[index], "ach_kill_1000", 1000);
		sync |= SteamCheckAch(_trivia[index], "ach_kill_2000", 2000);
		break;
	case TRIVIA_DEATHS:             stat_name = "Deaths"; break;
	case TRIVIA_CM_TRAVELED:        stat_name = "DistanceMoved"; break;
	case TRIVIA_PLAYTIME:           stat_name = "PlayTime"; break;
	case TRIVIA_MISSILES_DESTROYED: stat_name = "MissilesDestroyed"; break;
	case TRIVIA_MISSILES_EVADED:    stat_name = "MissilesEvaded"; break;
	default: break;
	}

	//std::string avg_stat_name = "Avg" + stat_name;
	//Add accurate session length here
	//double session_length = 1;

	if (SteamUserValid()) {
		if (m_pSteamUserStats->GetStat(stat_name.c_str(), &old_value))
		{
			m_pSteamUserStats->SetStat(stat_name.c_str(), (int32)(_trivia[index] + old_value));
			//m_pSteamUserStats->UpdateAvgRateStat(avg_stat_name.c_str(), (float)_trivia[index], session_length);
		}
	}
	if (sync)
		SteamSync();
}

void PlayerStats::ScoreCheckAch()
{
	bool sync = false;

	sync |= SteamCheckAch(Score(), "ach_score_10k", 10000);
	sync |= SteamCheckAch(Score(), "ach_score_100k", 100000);

	if (sync)
		SteamSync();
}

void PlayerStats::WorldCheckAch(int world_index, int zone_index)
{
	using namespace pyrodactyl;
	bool ach_unlocked = false;

	if (SteamUserValid())
	{
		if (world_index > 0)
		{
			std::string ach_id = "ach_lv_";

			if (world_index < 2)
				ach_id += NumberToString<int>(world_index);
			else if (world_index == 2)
				ach_id += "invalid";
			else
				ach_id += NumberToString<int>(world_index - 1);

			//This is the achievement for completing various levels
			m_pSteamUserStats->GetAchievement(ach_id.c_str(), &ach_unlocked);
			if (!ach_unlocked)
			{
				m_pSteamUserStats->SetAchievement(ach_id.c_str());
				ach_unlocked = true;
				SteamSync();
			}
		}

		//This checks the number of levels and zones since you lost your hat
		//At certain points, it gives you achievements
		if (_hat.wearing && _hat.level > -1 && _hat.zone > -1)
		{
			//Complete an entire level from start to finish without losing your hat
			if (world_index > _hat.level && _hat.zone == 0)
				m_pSteamUserStats->SetAchievement("ach_hattiquette");

			//Complete a zone without getting hit
			if (_hat.zone != zone_index)
				m_pSteamUserStats->SetAchievement("ach_watch_the_hat");

			if (world_index - _hat.level > 2)
				m_pSteamUserStats->SetAchievement("ach_elite_hat");
		}
	}
}

void PlayerStats::SkillCheckAch(PlayerSkill cat)
{
	bool sync = false;

	sync |= SteamCheckAch(_upgrade_count, "ach_upgrade_1", 1);
	sync |= SteamCheckAch(_upgrade_count, "ach_upgrade_10", 10);
	sync |= SteamCheckAch(_skill[cat], "ach_upgrade_all", EnvSkillMaxLevel(cat));

	if (sync)
		SteamSync();
}

void PlayerStats::WarrantyCheckAch()
{
	bool sync = false;

	sync |= SteamCheckAch(_warranty_count, "ach_warranty_1", 1);
	sync |= SteamCheckAch(_warranty_count, "ach_warranty_5", 5);

	if (sync)
		SteamSync();
}

void PlayerStats::WeaponCheckAch()
{
	bool sync = false;

	sync |= SteamCheckAch(_weapon_purchase_count, "ach_weapon_1", 1);
	sync |= SteamCheckAch(_weapon_purchase_count, "ach_weapon_10", 10);

	if (sync)
		SteamSync();
}

void PlayerStats::HatCheckAch()
{
	bool sync = false;

	sync |= SteamCheckAch(_hat.purchase_count, "ach_hat_1", 1);
	sync |= SteamCheckAch(_hat.purchase_count, "ach_hat_10", 10);

	if (sync)
		SteamSync();
}

void PlayerStats::WinCheckAch()
{
	//This is called when you win the game
	if (SteamUserValid())
	{
		//We got one guaranteed achievement to give (win game)
		m_pSteamUserStats->SetAchievement("ach_lv_7");

		//If the player managed to not get hit in the entire game, give them this achievement
		//you win, you beautiful bastard
		if (_hat.wearing && _hat.level == 0 && _hat.zone == 0)
			m_pSteamUserStats->SetAchievement("ach_hardest_hat");
	}

	SteamSync();
}

void PlayerStats::RepairCheckAch()
{
	bool sync = false;

	sync |= SteamCheckAch(_repair_count, "ach_repair_1", 1);
	sync |= SteamCheckAch(_repair_count, "ach_repair_5", 5);

	if (sync)
		SteamSync();
}

void PlayerStats::NewsCheckAch()
{
	++news_count;
	bool sync = false;

	sync |= SteamCheckAch(news_count, "ach_news_1", 2);

	if (sync)
		SteamSync();
}

void PlayerStats::TutorialCheckAch()
{
	++tutorial_count;
	bool sync = false;

	sync |= SteamCheckAch(tutorial_count, "ach_tutorial_1", 2);

	if (sync)
		SteamSync();
}

void OptionsCheckAch()
{
	bool sync = false;

	sync |= SteamCheckAch(1, "ach_opt_1", 1);

	if (sync)
		SteamSync();
}