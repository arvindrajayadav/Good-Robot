#ifndef ENV_H
#define ENV_H

#include "projectile.h"

struct RobotConfig;

enum EnvId
{
	ENV_FULLSCREEN,
	ENV_VSYNC,
	ENV_NOTILT,
	ENV_SHADER,
	ENV_SAFETY,
	ENV_CHEATS,
	ENV_FPS,
	ENV_FPSUNCAP,
	ENV_OCCLUDE,
	ENV_BBOX,
	ENV_NODIE,
	ENV_NODAMAGE,
	ENV_NOCLIP,
	ENV_LOS,
	ENV_AI,
	ENV_BUMP,
	ENV_SHOTS_RELATIVE,
	ENV_VOLUME_MUSIC,
	ENV_VOLUME_FX,
	ENV_MOUSE_SPEED,
	ENV_SHADOWS,
	ENV_GORG,
	ENV_KFA,
	ENV_INFO,
	ENV_CONTROLLER_ALT,
	ENV_RENDER_BACKGROUND,
	ENV_RENDER_WALLS,
	ENV_RENDER_GLOW,
	ENV_RENDER_DUST,
	ENV_RENDER_PARTICLES,
	ENV_RENDER_OVERLAY,
	ENV_RENDER_WIREFRAME,
	ENV_COUNT
};

struct EnvRule
{
	float       gravity;
	float       cursor_size;
	float       momentum_loss;
	float       bounce_velocity;
	float       acceleration;
	float       player_explosion_scale;
	float       screen_shake_max;
	float       screen_shake_player_dmg;
	float       screen_shake_explode;
	int         tile_variants;
	string      music_boss;
	string      music_win;
	string      music_dead;
	string      music_shop;
	float       pickup_range;
	int         multiplier_max;
	vector<int> multiplier_timeout;
};

//Values associated with one level of a game property
template <typename T>
struct PropertyData
{
	//The player needs to pay this much to go to the next level of the ability
	int cost;

	//The value at this level
	T val;

	PropertyData() { val = 1; cost = 100; }
};

bool                EnvValueb(EnvId id);
float               EnvValuef(EnvId id);
int                 EnvValuei(EnvId id);
void                EnvValueSetb(EnvId id, bool newval);
void                EnvValueSetf(EnvId id, float newval);
void                EnvValueSeti(EnvId id, int newval);

void                EnvInit();
char*               EnvMasterFile();
void                EnvReloadData();
bool                EnvHandleCommand(const vector<string>  &words);

const EnvRule&      Env();

PropertyData<int>   EnvShieldStrength(int power);
PropertyData<int>   EnvPlayerRefire(int power);
PropertyData<float> EnvPlayerSpeed(int power);
PropertyData<float> EnvDamageScaling(int power);
PropertyData<float> EnvVisionRadius(int power);

//Maximum level of things

class MachineInfo;

int                 EnvSkillMaxLevel(PlayerSkill index);
int                 EnvUpgradeCost(PlayerSkill index, int power, int upgrade_count);

//Power = Player level, Count = number of times warranty purchased already
int                 EnvWarrantyCost(int power, int count);

//Power = 1-ShieldPercent, Count = number of times robot was repaired
//Repair cost is float because shield percent is float
int                 EnvRepairCost(float power, int count);

int                 EnvShieldMax();
float               EnvPlayerVisionRadius();

const RobotConfig*  EnvRobotConfig(string type);
const RobotConfig*  EnvRobotConfig(int id);
bool                EnvIsRobotName(const char* name);
RobotType           EnvRobotIndexFromName(const string &name);
const char*         EnvRobotNameFromIndex(int id);
const char*         EnvRobotRollCall();

const Projectile*   EnvProjectileFromName(string name);
const Projectile*   EnvProjectileFromId(int id);
int                 EnvProjectileId(string name);
int                 EnvProjectileCount();
int                 EnvProjectileNext(int id_in, enum ProjectileSlot slot);
int                 EnvProjectileRandom(ProjectileSlot slot);

class Character&    EnvCharacter(int index);
int                 EnvCharacterIndexFromName(string name);
int                 EnvCharacterCount();

int                 EnvMapCount();

//vector<int>       EnvWeaponsStart();
//vector<int>       EnvWeaponsShop();
void                EnvProjectileRandomShop(ProjectileSlot slot, const Projectile* equipped, const int level_id, int &ret1, int &ret2);

void                EnvSetDifficulty(const bool val);

const MachineInfo*  EnvMachineFromName(string name);

const int           EnvGetHitpointsFromID(string id);
const int           EnvGetMoneyFromID(string id);
const int           EnvGetScoreFromID(string id);

const bool          EnvAchievementsEnabled();

#endif // ENV_H
