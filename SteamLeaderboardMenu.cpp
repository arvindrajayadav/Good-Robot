#include "master.h"
#include "SteamLeaderboardMenu.h"

using namespace pyrodactyl;

void SteamLeaderboardMenu::DownloadScores()
{
	//If we're already downloading scores, wait
	if (score_data.m_bLoading || !SteamUserValid())
		return;

	switch (sort)
	{
	case SORT_FRIENDS: request_type = k_ELeaderboardDataRequestFriends; break;
	case SORT_GLOBAL: request_type = k_ELeaderboardDataRequestGlobal; break;
	default: request_type = k_ELeaderboardDataRequestGlobalAroundUser;  break;
	}

	if (state == STATE_SCORE)
		score_data.DownloadEntries(leaderboard.handle_score, request_type, offset);
	else
		score_data.DownloadEntries(leaderboard.handle_kills, request_type, offset);

	rebuild = true;
}

void SteamLeaderboardMenu::Rebuild()
{
	SteamLeaderboard_t m_hSteamLeaderboard = 0;
	if (state == STATE_SCORE)
		m_hSteamLeaderboard = leaderboard.handle_score;
	else
		m_hSteamLeaderboard = leaderboard.handle_kills;

	if (!SteamUserValid())
	{
		notice.enabled = true;
		notice.text = "Invalid Steam user. Please launch the game from Steam.";
		HideMenu();
	}
	else if (!m_hSteamLeaderboard || score_data.m_bLoading)
	{
		notice.enabled = true;
		notice.text = "Loading...";
		HideMenu();
	}
	else if (score_data.m_bIOFailure)
	{
		notice.enabled = true;
		notice.text = "Network failure!";
		HideMenu();
	}
	else
	{
		if (score_data.m_nLeaderboardEntries == 0)
		{
			if (request_type != k_ELeaderboardDataRequestGlobalAroundUser)
			{
				notice.enabled = true;
				notice.text = "No scores for this leaderboard";
				HideMenu();
			}
			else
			{
				// Requesting for global scores around the user will return successfully with 0 results if the
				// user does not have an entry on the leaderboard
				notice.enabled = true;
				notice.text = SteamFriends()->GetPersonaName();
				notice.text += " does not have a score for this leaderboard";
				HideMenu();
			}
		}
		else
		{
			notice.enabled = false;
			HideMenu();

			for (int index = 0; index < score_data.m_nLeaderboardEntries; index++)
			{
				menu.element.at(index).SetText(score_data.m_leaderboardEntries[index].m_nGlobalRank,
					SteamFriends()->GetFriendPersonaName(score_data.m_leaderboardEntries[index].m_steamIDUser),
					score_data.m_leaderboardEntries[index].m_nScore);

				menu.element.at(index).Visible(true);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Load interface layout from file
//-----------------------------------------------------------------------------
void SteamLeaderboardMenu::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid("legend", node))
		legend.Load(node->first_node("legend"));

	if (NodeValid("ref", node))
		ref.Load(node->first_node("ref"));

	if (NodeValid("inc", node))
		inc.Load(node->first_node("inc"));

	if (NodeValid("notice", node))
		notice.Load(node->first_node("notice"));

	//This only loads the menu arrangement and not the contents
	if (NodeValid("menu", node))
		menu.Load(node->first_node("menu"));

	if (NodeValid("offset", node))
	{
		rapidxml::xml_node<char> *offnode = node->first_node("offset");

		if (NodeValid("inc", offnode))
			inc_offset.Load(offnode->first_node("inc"));

		if (NodeValid("dec", offnode))
			dec_offset.Load(offnode->first_node("dec"));

		if (NodeValid("reset", offnode))
			reset_offset.Load(offnode->first_node("reset"));
	}

	//Initialize the menu
	menu.Clear();
	for (int i = 0; i < score_data.k_nMaxLeaderboardEntries; ++i)
	{
		SteamLeaderboardButton sb;
		sb.Init(ref, nullptr, inc.RawPosX() * i, inc.RawPosY() * i);
		menu.element.push_back(sb);
	}
	menu.AssignPaths();

	if (NodeValid("switcher_state", node))
		switcher_state.Load(node->first_node("switcher_state"));

	if (NodeValid("switcher_sort", node))
		switcher_sort.Load(node->first_node("switcher_sort"));
}

void SteamLeaderboardMenu::HandleEvents()
{
	if (switcher_state.HandleEvents())
	{
		state = static_cast<LeaderboardType>(switcher_state.CurV().val);
		DownloadScores();
	}

	if (switcher_sort.HandleEvents())
	{
		sort = static_cast<SortingType>(switcher_sort.CurV().val);
		DownloadScores();
	}

	if (inc_offset.HandleEvents())
	{
		offset += score_data.k_nMaxLeaderboardEntries;
		DownloadScores();
	}

	if (score_data.m_leaderboardEntries[0].m_nGlobalRank > 2)
	{
		if (dec_offset.HandleEvents())
		{
			offset -= score_data.k_nMaxLeaderboardEntries;
			DownloadScores();
		}
	}

	if (reset_offset.HandleEvents())
	{
		offset = 0;
		DownloadScores();
	}

	menu.HandleEvents();
}

void SteamLeaderboardMenu::Draw()
{
	if (first_run)
	{
		DownloadScores();
		first_run = false;
	}

	if (rebuild)
	{
		Rebuild();

		if (!score_data.m_bLoading)
			rebuild = false;
	}

	legend.Draw();
	menu.Draw();
	notice.Draw();

	inc_offset.Draw();

	if (score_data.m_leaderboardEntries[0].m_nGlobalRank > 2)
		dec_offset.Draw();

	reset_offset.Draw();

	switcher_state.Draw();
	switcher_sort.Draw();
}

void SteamLeaderboardMenu::SetUI()
{
	ref.SetUI();
	inc.SetUI();

	legend.SetUI();
	menu.SetUI();
	notice.SetUI();

	inc_offset.SetUI();
	dec_offset.SetUI();
	reset_offset.SetUI();

	switcher_state.SetUI();
	switcher_sort.SetUI();
}