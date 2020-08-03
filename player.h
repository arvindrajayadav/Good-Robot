#ifndef PLAYER_H
#define PLAYER_H

#include "character.h"
#include "env.h"
#include "playerweapon.h"
#include "ZoneInfo.h"

struct Checkpoint
{
	int             map;
	int             zone;
	int             page;

	Checkpoint() {}
	Checkpoint(int m, int z, int p) : map(m), zone(z), page(p) {}
	bool   operator==(const Checkpoint& c) { if (map == c.map && zone == c.zone && page == c.page)  return true; return false; }
	bool   operator!=(const Checkpoint& c) { if (map != c.map || zone != c.zone || page != c.page)  return true; return false; }
};

enum
{
	PLAYER_WEAPON_PRIMARY,
	PLAYER_WEAPON_SECONDARY,
	PLAYER_WEAPON_COUNT
};

enum CompassState
{
	COMPASS_OFF, //We don't need to draw the compass, used only in special cases
	COMPASS_ON,  //Draw the compass pointing to the exit
	COMPASS_EXIT //If we are in the exit room, draw an exit symbol instead of an arrow
};

class PlayerStats
{
	int             _version;
	int             _timestamp_start;
	int             _timestamp_last;
	eGameMode       _game_mode;
	int             _character;
	Checkpoint      _checkpoint;
	int             _highest_map;
	GLrgba          _color;
	int             _skill[SKILL_COUNT];
	PlayerWeapon    _weapon[PLAYER_WEAPON_COUNT];
	bool            _ability[ABILITY_TYPES];
	long            _trivia[TRIVIA_COUNT];

	long             _score_points;
	long             _money;

	//The difficulty this player is playing at - true is normal, false is easy
	bool             _difficulty;

	//The current health of the player
	int             _shield_current;

	//The list of zones the player has completed in the current map
	std::vector<PlayerZoneInfo> _zone_info;

	//Do we respawn after we die?
	bool            _warranty;

	//The number of times we have purchased warranty
	int             _warranty_count;

	//Number of times we have repaired our robot
	int             _repair_count;

	//The total number of skills purchased
	int             _upgrade_count;

	//The total number of weapons purchased, used only for achievements
	int             _weapon_purchase_count;

	//Hat-related info that needs to be saved to file
	struct HatStats
	{
		//Are we wearing a hat
		bool wearing;

		//Sprite of hat
		std::string sprite;

		//Size of hat
		float size;

		//The number of times we've purchased a hat, used only for achievements
		int purchase_count;

		//The level and zone we got the hat in, used to calculate for how long does the player have a hat
		int level, zone;

		//The name of the hat
		std::string name;

		//The color of the hat
		GLrgba color;

		//Used to refresh store if player loses hate in the level and then goes to store
		bool lost;

		//If the player has already bought a hat, prevent them from buying it again
		bool purchased_this_zone;
	} _hat;

	//NOTE: Multiplier info is not saved to file because we want
	//the player to resume a saved game with the multiplier = 0

	int             _multiplier;
	int             _multiplier_timeout;

	void            TriviaCheckAch(PlayerTrivia index);
	void            SkillCheckAch(PlayerSkill cat);
	void            ScoreCheckAch();

public:
	void            WorldCheckAch(int world_index, int zone_index);
	void            WarrantyCheckAch();
	void            WeaponCheckAch();
	void            RepairCheckAch();
	void            NewsCheckAch();
	void            TutorialCheckAch();
	void            HatCheckAch();
	void            WinCheckAch();

	bool            Ability(int index);
	void            Ability(int index, bool val);

	int             Id() { return _timestamp_start; }
	int             LastPlayed() { return _timestamp_last; }

	int             FileVersion() { return _version; }

	eGameMode       GameMode() { return _game_mode; }
	void            GameModeSet(eGameMode d) { _game_mode = d; }

	GLrgba          Color() const { return _color; }
	int             Character() const { return _character; }

	long            Trivia(PlayerTrivia index) const { return _trivia[index]; }
	void            TriviaSet(PlayerTrivia index, int value) { _trivia[index] = value; TriviaCheckAch(index); }
	void            TriviaModify(PlayerTrivia index, int change) { _trivia[index] += change; TriviaCheckAch(index); }

	int             Skill(PlayerSkill index) const { return _skill[index]; }
	void            SkillSet(PlayerSkill index, int val);
	void            SkillUp(PlayerSkill skill);
	bool            SkillUpEligible(PlayerSkill skill) { return _skill[skill] < EnvSkillMaxLevel(skill); }

	Checkpoint      CheckpointGet() const { return _checkpoint; }
	void            CheckpointSet(Checkpoint location);

	long            Score() { return _score_points; }
	void            ScorePoints(int pts);

	int             Shields() const { return _shield_current; }
	void            ShieldsSet(int val);
	void            ShieldsModify(int change);
	float           ShieldsPercent();
	bool            ShieldsFull();
	void            RestoreShields();
	int             ShieldsMaxValue();

	void            EventRobotsKill(int count);
	void            EventMissileKill();

	int             Multiplier() { return _multiplier; }
	int             UpgradeCount() { return _upgrade_count; }

	long            Money() const { return _money; }
	void            MoneyGive(int val);

	void            WarrantySet(bool val) { _warranty = val; }
	bool            Warranty() { return _warranty; }

	void            WarrantyCountInc() { _warranty_count++; }
	int             WarrantyCount() { return _warranty_count; }

	void            RepairCountInc() { _repair_count++; }
	int             RepairCount() { return _repair_count; }

	void            WeaponPurchaseCountInc() { _weapon_purchase_count++; }
	int             WeaponPurchaseCount() { return _weapon_purchase_count; }

	void            HatPurchaseCountInc() { _hat.purchase_count++; }
	int             HatPurchaseCount() { return _hat.purchase_count; }

	void            Hat(bool status) { _hat.wearing = status; }
	bool            Hat() { return _hat.wearing; }

	void            HatPurchasedThisZone(bool status) { _hat.purchased_this_zone = status; }
	bool            HatPurchasedThisZone() { return _hat.purchased_this_zone; }

	void            HatLost(bool status) { _hat.lost = status; }
	bool            HatLost() { return _hat.lost; }

	void            HatLevel(int val) { _hat.level = val; }
	int             HatLevel() { return _hat.level; }

	void            HatZone(int val) { _hat.zone = val; }
	int             HatZone() { return _hat.zone; }

	void            HatSprite(std::string val) { _hat.sprite = val; }
	std::string     HatSprite() { return _hat.sprite; }

	void            HatName(std::string val) { _hat.name = val; }
	std::string     HatName() { return _hat.name; }

	void            HatSize(float val) { _hat.size = val; }
	float           HatSize() { return _hat.size; }

	void            HatColor(GLrgba val) { _hat.color = val; }
	GLrgba          HatColor() { return _hat.color; }

	PlayerWeapon*   Weapon(int index) { return &_weapon[index]; }

	void            ZoneAdd(const PlayerZoneInfo &pzi, const bool overwrite);
	bool            ZoneComplete(const int map_id, const int zone_id);

	void            Init(int character, eGameMode d);
	void            Kill(GLvector2 position, GLvector2 momentum);
	bool            Dead() { return _shield_current < 1; }

	void            DifficultySet(const bool val) { _difficulty = val; }
	bool            DifficultyGet() { return _difficulty; }

	float           VisionRadius();

	void            Save(string filename);
	bool            Load(string filename);
	void            Update();

	void            SaveHighScore();
};

bool              PlayerAccessActivated();
void              PlayerAddXp(int coins);
void			  PlayerDealDamage(int damage);
void			  PlayerScoreKill();
void			  PlayerScorePoints(int pts);
GLvector2         PlayerAim();
void              PlayerAutoDrive(GLvector2 direction);
bool              PlayerCollide(GLvector2 pos);
CompassState      PlayerCompass(float& angle);
void              PlayerDamage(int points);
void              PlayerDamage(int points, GLvector2 direction);
float             PlayerDeathFade();
bool              PlayerIgnore();
float             PlayerGatherDistance();

void              PlayerHatGive(string name, string sprite_name, float size, GLrgba color);
GLvector2         PlayerHead();
void              PlayerInit();
void              PlayerHatInit();
void              PlayerIncomingMissile(GLvector2 position);
GLvector2         PlayerMomentum();
void              PlayerOfferAccess(float distance, const char* message);
GLvector2         PlayerPosition();
GLvector2         PlayerOrigin();
void              PlayerOriginSet(GLvector2 new_pos);
void              PlayerRender();
void              PlayerReload();
int               PlayerShieldChance();
void              PlayerShove(GLvector2 force);
float             PlayerSize();
void              PlayerSpawn(GLvector2 pos);
PlayerStats*      Player();
void              PlayerUpdate();
const char*       PlayerZones();

#endif // PLAYER_H