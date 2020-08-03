/*-----------------------------------------------------------------------------

  Env.cpp

  Environment variables and flags.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "bodyparts.h"
#include "character.h"
#include "console.h"
#include "env.h"
#include "file.h"
#include "ini.h"
#include "fxmachine.h"
#include "player.h"
#include "projectile.h"
#include "random.h"
#include "resource.h"
#include "robot.h"
#include "system.h"
#include "GameProperty.h"
#include "XMLDoc.h"
#include "loaders.h"

using namespace pyrodactyl;

#define DELIMIT                 " ,:"
#define MAX_MSG_LEN             1024

#define COMMANDS                (sizeof (env_var) / sizeof (EnvVar))

#define ROBOTS_FILE             "robots.ini"
#define CHARACTERS_FILE         "characters.ini"
#define PROJECTILES_FILE        "projectiles.ini"
#define MACHINES_XML            "machines.xml"
#define PROPERTY_FILE_NORMAL    "core/data/gameplay.xml"
#define PROPERTY_FILE_EASY      "core/data/gameplay_easy.xml"

enum EnvType
{
	TYPE_BOOL,
	TYPE_INT,
	TYPE_FLOAT,
};

struct EnvValue
{
	bool          vbool;
	float         vfloat;
	int           vint;

	void          Set(float f) { vfloat = f; }
	void          Set(int i) { vint = i; }
	void          Set(bool b) { vbool = b; }
};

struct EnvVar
{
	char*           command;
	char*           name;
	EnvType         type;
	bool            save;
	bool            restricted;
};

//This list needs to match the enum in env.h. If you add one there, you
//much change this list as well.
static EnvVar   env_var[ENV_COUNT] =
{
	{ "fullscreen", "Fullscreen mode", TYPE_BOOL, true, false },
	{ "vsync", "Vsync", TYPE_BOOL, true, false },
	{ "tilt", "Disable tilt effect", TYPE_BOOL, true, false },
	{ "shader", "Vertex shader", TYPE_BOOL, true, false },
	{ "safety", "Weapon safety", TYPE_BOOL, true, false },
	{ "cheats", "Cheat shortcuts", TYPE_BOOL, false, false },
	{ "fps", "Show frame counter", TYPE_BOOL, false, false },
	{ "fpsuncap", "Framerate uncap", TYPE_BOOL, false, true },
	{ "occlude", "Occlusion", TYPE_BOOL, false, true },
	{ "bbox", "Show bounding boxes", TYPE_BOOL, false, true },
	{ "god", "Invulnerability", TYPE_BOOL, false, true },
	{ "nodamage", "Invulnerability", TYPE_BOOL, false, true },
	{ "noclip", "Noclip", TYPE_BOOL, false, true },
	{ "los", "Show LOS points", TYPE_BOOL, false, true },
	{ "ai", "Enemy AI", TYPE_BOOL, false, true },
	{ "bump", "Show bump checks", TYPE_BOOL, false, true },
	{ "shotrel", "Shot relative velocity", TYPE_BOOL, false, true },
	{ "music", "Music volume", TYPE_FLOAT, true, true },
	{ "sound", "Sound effects volume", TYPE_FLOAT, true, true },
	{ "mouse", "Mouse aiming speed", TYPE_FLOAT, true, true },
	{ "shadows", "Render shadows", TYPE_BOOL, false, true },
	{ "gorg", "Robot Invulnerability", TYPE_BOOL, false, true },
	{ "kfa", "Super damage", TYPE_BOOL, false, true },
	{ "info", "Info overlay", TYPE_BOOL, false, true },
	{ "controller_alt", "Alternate Controller Scheme", TYPE_BOOL, true, false },
	{ "render.background", "Render background", TYPE_BOOL, false, false },
	{ "render.walls", "Render walls", TYPE_BOOL, false, false },
	{ "render.glow", "Render glow", TYPE_BOOL, false, false },
	{ "render.dust", "Render dust", TYPE_BOOL, false, false },
	{ "render.particles", "Render particles", TYPE_BOOL, false, false },
	{ "render.overlay", "Render overlay", TYPE_BOOL, false, false },
	{ "render.wireframe", "Render wireframe", TYPE_BOOL, false, false },
};

class CostFactor
{
	int base, count, power;

public:
	CostFactor() { base = 100; count = 100; power = 100; }

	void Load(rapidxml::xml_node<char> *node)
	{
		LoadNum(base, "base", node);
		LoadNum(count, "count", node);
		LoadNum(power, "power", node);
	}

	int Cost(float Power, int Count) { return base + (int)((float)power * Power) + (count * Count); }
};

static GameProperty<int>                  shield_strength;
static GameProperty<int>                  player_refire;
static GameProperty<float>                player_speed;
static GameProperty<float>                damage_scaling;
static GameProperty<float>                vision_radius;

static EnvValue                           var[ENV_COUNT];
static vector<string>                     bot_name;
static vector<RobotConfig>                bot_config;
static vector<Character>                  character;
static vector<Projectile>                 projectile;
static vector<MachineInfo>                machine;
static string                             bot_roll_call;
static EnvRule                            env;
static int                                map_count;
static CostFactor                         warranty_cost;
static CostFactor                         repair_cost;
static unordered_map<string, int>         hitpoints_table, money_table, score_table;
static bool                               achievements_enabled;

//A different weapon list for every level
typedef vector<int> WeaponList;
static vector<WeaponList>                 weapons_shop;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

const bool EnvAchievementsEnabled()
{
	return achievements_enabled;
}

vector<int> weapon_ids(string list)
{
	vector<string>  weapon_names;
	vector<int>     result;

	weapon_names = StringSplit(list, " ,");
	for (unsigned i = 0; i < weapon_names.size(); i++) {
		result.push_back(EnvProjectileId(weapon_names[i]));
	}
	if (result.empty())
		result.push_back(0);
	return result;
}

static void do_machine_inventory()
{
	XMLDoc      mach_file(ResourceLocation(MACHINES_XML, RESOURCE_DATA));
	MachineInfo mi;

	if (!mach_file.ready())
		return;

	rapidxml::xml_node<char> *node = mach_file.Doc()->first_node(0, 0, false);
	if (NodeValid(node))
	{
		for (rapidxml::xml_node<char> *n = node->first_node(); n != nullptr; n = n->next_sibling())
		{
			string name = n->name();
			//Console("Loading machine '%s'", name.c_str());
			mi.Load(n);
			machine.push_back(mi);
		}
	}
}

static int do_map_inventory()
{
	int       count;
	XMLDoc    level_file(ResourceLocation(LEVELS_XML, RESOURCE_DATA));

	if (!level_file.ready())
		return 0;

	rapidxml::xml_node<char> *node = level_file.Doc()->first_node("levels");
	if (node == nullptr)
		return 0;

	count = 0;
	for (rapidxml::xml_node<char> *n = node->first_node(); n != nullptr; n = n->next_sibling(), count++);

	return count;
}

static void do_projectile_inventory(iniFile &ini)
{
	Projectile      p;
	int							start = SystemTick();

	projectile.clear();
	for (unsigned i = 0; i < ini.SectionCount(); i++) {
		if (ini.SectionName(i).length() > 1) {
			p.Init(ini, ini.SectionName(i));
			projectile.push_back(p);
		}
	}
	Console("Loaded %d projectiles in %dms.", projectile.size(), SystemTick() - start);
}

static string var_print(int id)
{
	string    result;
	char      temp[20];

	if (env_var[id].type == TYPE_BOOL)
		result = var[id].vbool ? "Off" : "On";
	else if (env_var[id].type == TYPE_INT) {
		sprintf(temp, "%d", var[id].vint);
		result = temp;
	}
	else if (env_var[id].type == TYPE_FLOAT) {
		sprintf(temp, "%1.2f", var[id].vfloat);
		result = temp;
	}
	return result;
}

static void var_changed(int id)
{
	if (!env_var[id].save)
		return;
	iniFile   ini;
	string    filename;

	ini.Open(SystemConfigFile());
	if (env_var[id].type == TYPE_BOOL)
		ini.IntSet("Settings", env_var[id].command, var[id].vbool ? 1 : 0);
	else
		ini.StringSet("Settings", env_var[id].command, var_print(id));
}

static void var_read(int id, string str, bool save = true)
{
	if (env_var[id].type == TYPE_BOOL)
		var[id].Set(atoi(str.c_str()) > 0);
	else if (env_var[id].type == TYPE_INT)
		var[id].Set(atoi(str.c_str()));
	else if (env_var[id].type == TYPE_FLOAT)
		var[id].Set((float)atof(str.c_str()));
	if (save)
		var_changed(id);
}

static vector<int> load_multiplier_timeout(string input)
{
	vector<string>  values;
	vector<int>     result;

	values = StringSplit(input, " ,");
	//If the ini file gives less than 2 multipliers, then it's screwed up.
	//Rather than crash, print an error and fill the list with dummy values.
	if (values.size() < 2) {
		Console("Invalid multiplier values given for MultiplierTimeout.");
		result.push_back(1000);
		result.push_back(1000);
		result.push_back(1000);
		return result;
	}
	for (unsigned i = 0; i < values.size(); i++) {
		result.push_back(atoi(values[i].c_str()));
	}
	return result;
}

static void do_robot_inventory(iniFile &ini)
{
	vector<string>  parse;

	bot_name.clear();
	bot_config.clear();
	bot_roll_call = "";
	//First we load in just the NAMES of the robots. We get ALL the names
	//before we look at the data, since robots refer to each other by name
	//and we need that data available.
	for (unsigned i = 0; i < ini.SectionCount(); i++) {
		if (ini.SectionName(i).length() > 1)
			bot_name.push_back(StringToLower(ini.SectionName(i)));
	}
	for (unsigned i = 0; i < bot_name.size(); i++) {
		if (i)
			bot_roll_call += ", ";
		bot_roll_call += bot_name[i];
	}
	//Now load the actual data to go with the name.
	for (unsigned i = 0; i < bot_name.size(); i++) {
		RobotConfig   rc;

		rc.Load(ini, bot_name[i]);
		bot_config.push_back(rc);
	}
}

/*-----------------------------------------------------------------------------
Set / get of environment variables.
-----------------------------------------------------------------------------*/

//string EnvRobotRollCall () { 	return bot_roll_call; }
//

bool        EnvValueb(EnvId id)
{
	return var[id].vbool;
}

float       EnvValuef(EnvId id)
{
	return var[id].vfloat;
}

int         EnvValuei(EnvId id)
{
	return var[id].vint;
}

void        EnvValueSetb(EnvId id, bool newval)
{
	if (var[id].vbool == newval)
		return;
	var[id].Set(newval);
	var_changed(id);
}

void        EnvValueSetf(EnvId id, float newval)
{
	if (var[id].vfloat == newval)
		return;
	var[id].Set(newval);
	var_changed(id);
}

void        EnvValueSeti(EnvId id, int newval)
{
	if (var[id].vint == newval)
		return;
	var[id].Set(newval);
	var_changed(id);
}

/*-----------------------------------------------------------------------------
Various mechanical values.
-----------------------------------------------------------------------------*/

//vector<int>       EnvWeaponsStart() { return weapons_start; }
int                 EnvMapCount() { return map_count; }
const EnvRule&      Env() { return env; }

vector<int>         EnvWeaponsShop(const int level_index)
{
	//If this is true, we messed up somewhere
	if (!weapons_shop.empty()) {
		if (level_index < weapons_shop.size())
			return weapons_shop.at(level_index);

		//This means the level index does not have a shop list
		//associated with it, so we return the list of the final level we have
		return weapons_shop.at(weapons_shop.size() - 1);
	}

	vector<int> empty_list;
	return empty_list;
}

int EnvSkillMaxLevel(PlayerSkill index)
{
	int max_level = 0;
	switch (index) {
	case SKILL_SHOT_POWER:    max_level = damage_scaling.Size() - 1; break;
	case SKILL_SHOT_SPEED:    max_level = player_refire.Size() - 1; break;
	case SKILL_SPEED:         max_level = player_speed.Size() - 1; break;
	case SKILL_SHIELDS:       max_level = shield_strength.Size() - 1; break;
	case SKILL_VISION_RADIUS: max_level = vision_radius.Size() - 1; break;
	default:break;
	}
	return max_level;
}

int EnvUpgradeCost(PlayerSkill index, int power, int upgrade_count)
{
	int cost = 0;
	switch (index) {
	case SKILL_SHOT_POWER:    cost = EnvDamageScaling(power).cost;    cost += (int)(damage_scaling.UpgradeCountFactor() * (float)upgrade_count * (float)cost); break;
	case SKILL_SHOT_SPEED:    cost = EnvPlayerRefire(power).cost;     cost += (int)(player_refire.UpgradeCountFactor() * (float)upgrade_count * (float)cost); break;
	case SKILL_SPEED:         cost = EnvPlayerSpeed(power).cost;      cost += (int)(player_speed.UpgradeCountFactor() * (float)upgrade_count * (float)cost); break;
	case SKILL_SHIELDS:       cost = EnvShieldStrength(power).cost;   cost += (int)(shield_strength.UpgradeCountFactor() * (float)upgrade_count * (float)cost); break;
	case SKILL_VISION_RADIUS: cost = EnvShieldStrength(power).cost;   cost += (int)(vision_radius.UpgradeCountFactor() * (float)upgrade_count * (float)cost); break;
	default:break;
	}

	return cost;
}

int EnvWarrantyCost(int power, int count)
{
	return warranty_cost.Cost((float)power, count);
}

int EnvRepairCost(float power, int count)
{
	return repair_cost.Cost(power, count);
}

int EnvShieldMax()
{
	return shield_strength.Max();
}

float EnvPlayerVisionRadius()
{
	return EnvVisionRadius(Player()->Skill(SKILL_VISION_RADIUS)).val;
}

PropertyData<int> EnvShieldStrength(int power)
{
	return shield_strength.Get(power);
}

PropertyData<int> EnvPlayerRefire(int power)
{
	return player_refire.Get(power);
}

PropertyData<float> EnvPlayerSpeed(int power)
{
	return player_speed.Get(power);
}

PropertyData<float> EnvDamageScaling(int power)
{
	return damage_scaling.Get(power);
}

PropertyData<float> EnvVisionRadius(int power)
{
	return vision_radius.Get(power);
}

const RobotConfig* EnvRobotConfig(string type)
{
	return &bot_config[EnvRobotIndexFromName(type)];
}

const RobotConfig* EnvRobotConfig(int id)
{
	return &bot_config[id];
}

bool EnvIsRobotName(const char* name)
{
	RobotType t;

	t = (RobotType)EnvRobotIndexFromName(name);
	return t != ROBOT_INVALID;
}

RobotType EnvRobotIndexFromName(const string &name_in)
{
	string name = StringToLower(name_in);
	for (unsigned i = 0; i < bot_name.size(); i++)
		if (!_stricmp(bot_name[i].c_str(), name.c_str()))
			return (RobotType)i;
	return ROBOT_INVALID;
}

const char* EnvRobotNameFromIndex(int id)
{
	if (id < 0 || id >= (int)bot_name.size())
		return "Unknown";
	return bot_name[id].c_str();
}

const int EnvGetHitpointsFromID(string id)
{
	if (hitpoints_table.count(id) > 0)
		return hitpoints_table.at(id);

	return 1;
}

const int EnvGetMoneyFromID(string id)
{
	if (money_table.count(id) > 0)
		return money_table.at(id);

	return 1;
}

const int EnvGetScoreFromID(string id)
{
	if (score_table.count(id) > 0)
		return score_table.at(id);

	return 1;
}

char* EnvMasterFile()
{
	return GAMEPLAY_FILE;
}

const char* EnvRobotRollCall()
{
	return bot_roll_call.c_str();
}

bool EnvHandleCommand(const vector<string>  &words)
{
	if (words.empty())
		return false;
	//see if the user is setting a value
	for (int i = 0; i < COMMANDS; i++) {
		//Don't allow restricted commands unless cheats are on.
		if (!EnvValueb(ENV_CHEATS) && env_var[i].restricted)
			continue;
		if (_stricmp(words[0].c_str(), env_var[i].command))
			continue;
		if (words.size() > 1)
			var_read(i, words[1]);
		else if (env_var[i].type == TYPE_BOOL) //typing the name of a bool flips it
			var[i].Set(!var[i].vbool);
		if (env_var[i].type == TYPE_BOOL)
			Console("%s %s", env_var[i].name, var[i].vbool ? "ON" : "OFF");
		else {
			string  printval;

			printval = var_print(i);
			Console("%s = %s", env_var[i].name, printval.c_str());
		}
		var_changed(i);
	}
	return false;
}

void EnvLoadDifficulty()
{
	XMLDoc prop_file;
	if (Player()->DifficultyGet())
		prop_file.Load(PROPERTY_FILE_NORMAL);
	else
		prop_file.Load(PROPERTY_FILE_EASY);

	if (prop_file.ready()) {
		rapidxml::xml_node<char> *node = prop_file.Doc()->first_node("property");

		if (NodeValid(node)) {
			if (NodeValid("ShieldStrength", node))
				shield_strength.Load(node->first_node("ShieldStrength"));

			if (NodeValid("PlayerRefire", node))
				player_refire.Load(node->first_node("PlayerRefire"));

			if (NodeValid("PlayerSpeed", node))
				player_speed.Load(node->first_node("PlayerSpeed"));

			if (NodeValid("DamageScaling", node))
				damage_scaling.Load(node->first_node("DamageScaling"));

			if (NodeValid("VisionRadius", node))
				vision_radius.Load(node->first_node("VisionRadius"));

			if (NodeValid("warranty", node))
				warranty_cost.Load(node->first_node("warranty"));

			if (NodeValid("repair", node))
				repair_cost.Load(node->first_node("repair"));

			achievements_enabled = true;
			if (NodeValid("achievements", node))
				LoadBool(achievements_enabled, "enabled", node->first_node("achievements"));

			//Separate weapon list for each level
			weapons_shop.clear();
			if (NodeValid("shop", node)) {
				for (rapidxml::xml_node<char> *n = node->first_node("shop")->first_node(); n != nullptr; n = n->next_sibling())
				{
					std::string str;
					LoadStr(str, "list", n);
					WeaponList l = weapon_ids(str);
					weapons_shop.push_back(l);
				}
			}

			hitpoints_table.clear();
			if (NodeValid("RobotHP", node)) {
				for (rapidxml::xml_node<char> *n = node->first_node("RobotHP")->first_node(); n != nullptr; n = n->next_sibling())
				{
					std::string str;
					int val = 1;

					if (LoadStr(str, "id", n) && LoadNum(val, "val", n))
						hitpoints_table[str] = val;
				}
			}

			money_table.clear();
			if (NodeValid("RobotMoney", node)) {
				for (rapidxml::xml_node<char> *n = node->first_node("RobotMoney")->first_node(); n != nullptr; n = n->next_sibling())
				{
					std::string str;
					int val = 1;

					if (LoadStr(str, "id", n) && LoadNum(val, "val", n))
						money_table[str] = val;
				}
			}

			score_table.clear();
			if (NodeValid("RobotScore", node)) {
				for (rapidxml::xml_node<char> *n = node->first_node("RobotScore")->first_node(); n != nullptr; n = n->next_sibling())
				{
					std::string str;
					int val = 1;

					if (LoadStr(str, "id", n) && LoadNum(val, "val", n))
						score_table[str] = val;
				}
			}
		}
	}
}

void EnvReloadData()
{
	iniFile         ini;
	string          filename;

	filename = ResourceLocation(PROJECTILES_FILE, RESOURCE_DATA);
	Console("GameReloadData: Loading settings from %s", filename.c_str());
	ini.Open(filename);
	do_projectile_inventory(ini);

	EnvLoadDifficulty();

	filename = ResourceLocation(ROBOTS_FILE, RESOURCE_DATA);
	Console("GameReloadData: Loading settings from %s", filename.c_str());
	ini.Open(filename);
	do_robot_inventory(ini);
	do_machine_inventory();
	filename = ResourceLocation(GAMEPLAY_FILE, RESOURCE_DATA);
	Console("GameReloadData: Loading settings from %s", filename.c_str());
	//Now that we have ALL projectiles and robots, go back and fill in
	//the robot weapon data.
	for (unsigned i = 0; i < bot_config.size(); i++) {
		for (unsigned w = 0; w < bot_config[i].weapons.size(); w++) {
			string payload = bot_config[i].weapons[w].payload_name;

			bot_config[i].weapons[w].projectile_id = EnvProjectileId(payload);
			bot_config[i].weapons[w].robot_id = EnvRobotIndexFromName(payload);
		}
	}

	ini.Open(filename);
	//Load the various rule values.
	env.momentum_loss = ini.FloatGet("Gameplay", "MomentumLoss");
	env.cursor_size = ini.FloatGet("Gameplay", "CursorSize");
	env.cursor_size = clamp(env.cursor_size, 0.1f, 1.0f);
	env.screen_shake_max = ini.FloatGet("Gameplay", "ScreenShakeMax");
	env.screen_shake_player_dmg = ini.FloatGet("Gameplay", "ScreenShakePlayerDmg");
	env.screen_shake_explode = ini.FloatGet("Gameplay", "ScreenShakeExplode");

	env.bounce_velocity = ini.FloatGet("Gameplay", "BounceVelocity");
	env.acceleration = ini.FloatGet("Gameplay", "Acceleration");
	env.player_explosion_scale = ini.FloatGet("Gameplay", "PlayerExplosionScale");
	env.pickup_range = ini.FloatGet("Gameplay", "PickupRange");
	env.tile_variants = ini.IntGet("Gameplay", "TileVariants");
	env.music_boss = ini.StringGet("Audio", "MusicBoss");
	env.music_win = ini.StringGet("Audio", "MusicWin");
	env.music_dead = ini.StringGet("Audio", "MusicDead");
	env.music_shop = ini.StringGet("Audio", "MusicShop");
	env.multiplier_timeout = load_multiplier_timeout(ini.StringGet("Gameplay", "MultiplierTimeout"));
	env.multiplier_max = env.multiplier_timeout.size() - 1;

	//Load the characters
	filename = ResourceLocation(CHARACTERS_FILE, RESOURCE_DATA);
	Console("GameReloadData: Loading characters from %s", filename.c_str());
	character.clear();
	ini.Open(filename);
	for (unsigned i = 0; i < ini.SectionCount(); i++) {
		Character ch;

		if (ini.SectionName(i).empty())
			continue;
		ch.Init(ini, ini.SectionName(i));
		character.push_back(ch);
	}
	Console("GameReloadData: Loaded %d characters", character.size());
	PlayerReload();
}

int EnvCharacterCount() { return character.size(); }

Character& EnvCharacter(int index)
{
	return character[index];
}

int EnvCharacterIndexFromName(string name)
{
	string  name_lwr = StringToLower(name);

	for (unsigned i = 0; i < character.size(); i++) {
		string  char_lwr = StringToLower(character[i]._name);
		if (char_lwr.compare(name_lwr) == 0)
			return i;
	}
	Console("Unknown character '%s'", name.c_str());
	return 0;
}

void EnvInit()
{
	iniFile   ini;
	string    filename;

	EnvReloadData();

	map_count = do_map_inventory();

	ini.Open(SystemConfigFile());
	if (!ini.BoolGet("Settings", "Setup")) {
		ini.BoolSet("Settings", "Setup", true);
		EnvValueSetf(ENV_VOLUME_MUSIC, 0.5f);
		EnvValueSetf(ENV_VOLUME_FX, 1.0f);
		EnvValueSetf(ENV_MOUSE_SPEED, 1.0f);
		EnvValueSetb(ENV_FULLSCREEN, false);
		EnvValueSetb(ENV_NOTILT, false);
		ini.Open(SystemConfigFile());
	}
	for (unsigned i = 0; i < ENV_COUNT; i++) {
		if (env_var[i].save)
			var_read(i, ini.StringGet("Settings", env_var[i].command), false);
	}
	var[ENV_SHADER].Set(true);
	var[ENV_OCCLUDE].Set(true);
	var[ENV_SHADOWS].Set(true);
	var[ENV_SHADOWS].Set(true);
	for (int i = ENV_RENDER_BACKGROUND; i <= ENV_RENDER_OVERLAY; i++)
		var[i].Set(true);
#ifdef _DEBUG
	var[ENV_CHEATS].Set(true);
	var[ENV_FPS].Set(true);
	var[ENV_FPSUNCAP].Set(false);
	var[ENV_INFO].Set(false);
	var[ENV_AI].Set(false);
	var[ENV_NODIE].Set(true);
	//var[ENV_NOCLIP].Set (true);
	var[ENV_BBOX].Set(false);
	//var[ENV_RENDER_WALLS].Set (false);
	//var[ENV_SHADOWS].Set (false);
	//var[ENV_NOAMMO].Set (true);
#else
	var[ENV_NOCLIP].Set(false);
	var[ENV_FPS].Set(false);
	var[ENV_AI].Set(true);
#endif
}

const Projectile* EnvProjectileFromId(int id)
{
	return &projectile[id];
}

int EnvProjectileCount()
{
	return projectile.size();
}

const Projectile* EnvProjectileFromName(string name_in)
{
	string    name = StringToLower(name_in);

	for (unsigned i = 0; i < projectile.size(); i++) {
		if (name.compare(StringToLower(projectile[i]._name)) == 0)
			return &projectile[i];
	}
	return &projectile[0];
}

int EnvProjectileId(string name_in)
{
	string    name = StringToLower(name_in);

	for (unsigned i = 0; i < projectile.size(); i++) {
		if (name.compare(StringToLower(projectile[i]._name)) == 0)
			return i;
	}
	return 0;
}

//Returns the next available weapon for the given slot, or echoes back
//the given id if none found. Really only useful in debugging.
int EnvProjectileNext(int id_in, ProjectileSlot slot)
{
	id_in = abs(id_in) % projectile.size();
	for (unsigned i = 0; i < projectile.size(); i++) {
		int     id = (i + id_in + 1) % projectile.size();
		if (projectile[id]._slot == slot)
			return id;
	}
	return id_in;
}

//Return the id of a randomly selected weapon for the given slot.
//Only really useful until we have a system to acquire weapons.
int EnvProjectileRandom(ProjectileSlot slot)
{
	vector<int>   list;

	for (unsigned i = 0; i < projectile.size(); i++) {
		if (projectile[i]._slot == slot)
			list.push_back(i);
	}
	return list[RandomVal(list.size())];
}

template<class bidiiter>
bidiiter random_unique(bidiiter begin, bidiiter end, size_t num_random) {
	size_t left = std::distance(begin, end);
	while (num_random--) {
		bidiiter r = begin;
		std::advance(r, RandomVal() % left);
		std::swap(*begin, *r);
		++begin;
		--left;
	}
	return begin;
}

//Returns two random weapon IDs that the player can buy
//Used to populate the store with random weapons from that level's list of weapons
void EnvProjectileRandomShop(ProjectileSlot slot, const Projectile* equipped, const int level_id, int &ret1, int &ret2)
{
	vector<int>   list;
	vector<int>   level_shop = EnvWeaponsShop(level_id);

	for (unsigned i = 0; i < level_shop.size(); i++) {
		Projectile* p = &projectile[level_shop[i]];

		if (p->_slot == slot && p->_id != equipped->_id)
			list.push_back(level_shop[i]);
	}

	random_unique(list.begin(), list.end(), 2);

	//No values in list
	if (list.empty())
		return;
	else if (list.size() == 1)
	{
		ret1 = list[0];
		ret2 = list[0];
	}
	else
	{
		ret1 = list[0];
		ret2 = list[1];
	}
}

void EnvSetDifficulty(const bool val)
{
	Player()->DifficultySet(val);
	EnvLoadDifficulty();
}

const MachineInfo*   EnvMachineFromName(string name_in)
{
	string    name = StringToLower(name_in);

	for (unsigned i = 0; i < machine.size(); i++) {
		if (name.compare(StringToLower(machine[i].Name())) == 0)
			return &machine[i];
	}
	return NULL;
	//return &machine[0];
}