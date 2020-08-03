#ifndef ROBOT_H
#define ROBOT_H

#include "sprite.h"
#include "bodyparts.h"

#define ROBOT_VISION_DISTANCE   PAGE_HALF
#define BOSS_DIE_TIME           3000
#define MAX_PARTS               7
#define SHOOT_ZONE              0.5f
#define LOS_STEP                0.1f
#define SHIELD_PULSE            20    //frames
#define SMOKE_INTERVAL          250   //Milliseconds
#define KICK_RECOVERY           0.9f
#define DEFAULT_SPOT_DISTANCE   10
#define ROBOT_INVALID           (RobotType)-1

enum ePickupType;

struct PewPew
{
	const class Projectile*   p_info;
	int                       robot_id;
	int                       send_time;
	bool                      aim_predicted;
	int                       body_part;
};

struct Weapon
{
	string        payload_name;   //The string of the projectile or robot this weapon will fire.
	int           projectile_id;  //The index of the projectile. Use EnvProjectileFromId ().
	int           robot_id;       //If this is NOT (-1), then this weapon shoots robots instead of bullets.
	int           body_part;      //Segment of the body it's attached to.
	int           volley;         //How many shots we group together.
	int           cooldown;       //Game tick when the weapon can fire again.
	int           next_fire;      //Used by robots. Next GameTick () when the weapon is ready again.
};

enum
{
	AI_IDLE,
	AI_FORWARD,
	AI_REVERSE,
	AI_SIDE,
	AI_WANDER,
	AI_APPROACH,
	AI_HOLD,
};

enum AiCore
{
	AI_TEST = -1,
	AI_BEELINE = 0,
	AI_WALK,
	AI_POUNCE,
	AI_SENTRY,
	AI_HITNRUN,
	AI_ORBIT,
	AI_TUNNEL,
	AI_GUARD,
};

enum eEyeMove
{
	EYEMOVE_FIXED,
	EYEMOVE_SWEEP,
	EYEMOVE_SCAN,
	EYEMOVE_PLAYER,
	EYEMOVE_HEADING
};

enum eLauncherClass
{
	LAUNCHER_NONE,
	LAUNCHER_BASIC,
	LAUNCHER_HOMING,
	LAUNCHER_CLUSTER,
	LAUNCHER_ROBOT,
	LAUNCHER_NUKE,
};

enum eBodyClass
{
	RBODY_FIXED,      //Boring unmoving sprite
	RBODY_SQUID,      //Three parts tril behind, fanned out.
	RBODY_SNAKE,      //A trail of parts.
	RBODY_WORM,       //A trail of parts that sticks to the ground.
	RBODY_JELLY,      //Fixed upper body, lower body "swings" according to movement.
	RBODY_TURRET,     //Fixed body, with a rotating "gun" on each side.
};

enum eMeleeClass
{
	MELEE_BITE,
	MELEE_SHOVE,
	MELEE_EXPLODE,
};

enum AiIdle
{
	AI_IDLE_WAIT,
	AI_IDLE_BURROW,
	AI_IDLE_HANG,
};

struct Weakpoint
{
	GLvector2         position;
	GLvector2		  shake;
	GLrgba            color;
	SpriteEntry       sprite;
	float             size;
	int               shake_stop;
};

struct RobotConfig
{
	string            type;               //From the settings file
	string            name;               //Name used in HP bar - for bosses.
	string            proximity_warning;  //Sound to use as a warning when it approaches.
	GLvector2         eye_offset;         //The offset of the eye from the body origin.
	GLrgba            body_color;         //Default color of the running lights.
	GLrgba            eye_color;          //Color of the eye.
	AiCore            ai_core;            //Enumerator indicating which Ai to use.
	AiIdle            ai_idle;            //What this robot does while waiting for the player.
	eLauncherClass    launcher;           //What kind of missiles to fire.
	eBodyClass        body;               //Style of body: snake, jelly, etc.
	eMeleeClass       melee_class;        //What type of melee attacks: Shoving, exploding, etc.

	SpriteEntry       parts[MAX_PARTS];   //Index of body in sprite sheet.
	int								part_count;
	RobotType         launch_robot;       //If this thing lanches robots, this defines what kind.
	int               max_children;       //If this launches other robots, this is the limit of how many can be active at aonce.
	int               score_value;        //How many points you get for killing one of these.
	int               xp_value;           //How much XP this robot will drop.
	int               melee_damage;       //Damage output for explosions and melee.
	int               hitpoints;          //Base hitpoints.
	int								child_death_damage;	//If one of our children dies, how much damage do we take?
	int               armor;              //Damage reduction. 1 armor cancels 1 damage.
	SpriteEntry       eye;                //Index of eye in sprite sheet.
	SpriteEntry       iris;               //Index of eye in sprite sheet.
	int               attack_predict;     //0=never, 1=always, n=every nth shot is predicted.
	float             attack_range;       //Desired attack range. Usage depends on AiCore.
	int               refire_melee;       //How often we can damage the player with melee.
	float             melee_shove_power;  //How much the robot shoves you back when it hits you
	string            sound_see;
	string            sound_hit;
	string            sound_die;
	int               eye_movement;       //Type of movement logic to use.
	int               legs;               //How many legs this thing has.
	bool              is_boss;
	bool              is_final_boss;			//True if the death of this robot shoud end the game on the next level load.
	bool              has_weakpoints;     //True if the main body of the robot is invulnerable to damage.
	bool              warn_proximity;     //True if the robot should beep as it gets closer.
	bool              is_explosive;
	bool							is_dependant;		  	//If the parent robot dies, this robot should explode.
	bool							is_follower;				//If true, chase your parent robot instead of player.
	bool							is_invulnerable;		//If true, the robot can't be hurt by projectiles or explosions.
	float							inertia;						//How hard it is for the robot to change direction. Higher values are smoother movement, worse manuvering.
	float             eye_size;           //Eye size as portion of body size.
	float             walk_stride;        //Distance from body to knee.
	float             walk_height;        //Distance from knee to floor.
	float             walk_crouch;        //Distance to floor when crouching.
	float             walk_stand;         //Distance from body to floor when standing.
	float             size;               //Size of body.
	float             speed;              //Movement speed.
	float             spot_distance;      //How close the player must before bot becomes alerted.
	string            drop;               //The id of the drop to use from the drop table.

	float             screen_shake_alert; //BOSS ONLY: Screenshake when alerted
	float             screen_shake_bore;  //BOSS ONLY: Screenshake when digging underground
	vector<Weapon>    weapons;            //The last of weapons available to this robot type.
	vector<Weakpoint> weakpoint;
	vector<int>       weapon_drops;       //A list of possible weapon id's to drop on death.

	eBodyClass        BodyFromString(string str);
	void              PartsFromString(string str, SpriteEntry* parts, bool boss);
	AiCore            CoreFromString(string str);
	AiIdle            IdleFromString(string str);
	eMeleeClass       MeleefromString(string str);
	eEyeMove          EyeMoveFromString(string str);
	vector<int>       WeaponsFromString(string list);
	bool              WeakpointFromString(string data, Weakpoint& w);
	Weapon            WeaponFromString(string data);
	void              EyeFromString(string data);
	void              VoiceFromInt(int voice);

	void              Load(class iniFile& ini, string section);
	void              LoadTemplate(class iniFile& ini, string section);
};

enum
{
	MOVE_FORWARD,
	MOVE_REVERSE,
	MOVE_SIDE,
	MOVE_WANDER,
	MOVE_APPROACH,
	MOVE_COUNT
};

class Robot
{
	GLvector2           _position;          //Current location in world coords.
	GLvector2           _at_player;         //A vector pointing at the player.
	GLvector2           _at_movement;       //A vector pointed which way we're moving.
	GLvector2           _shove;             //Accumulated pushing from other robots to prevent deathball stacking.
	GLvector2           _player_predicted;  //Where to aim to hit the moving player.
	GLvector2           _exhaust_point;     //When dying, the point where smoke comes out.
	GLvector2           _bore_point;        //When tunneling, this was the last non-underground position;
	GLvector2						_bob;
	GLvector2           _launch_inertia;    //Forced movement, imposed by one bot spawning another and then "shoving" the child away from it to avoid clustering.
	GLvector2           _inertia;						//The vector of our current movement intertia.
	GLrgba              _body_color;        //Base of the running lights.
	vector<PewPew>      _pew_pew;           //A list of projectiles to shoot.
	string              _type;              //Type of robot.
	const RobotConfig*  _config;            //A struct holding all the properties of this robot type.
	GLbbox2             _bbox;              //Bounding box for collision-checking.
	SpriteUnit          _sprite[MAX_PARTS];
	GLvector2           _anchor[MAX_PARTS * 2];
	int                 _body_part_count;
	int									_pain_sprite;				//The last segment of the body that was damaged.

	int                 _id;                //A number unique to each robot. Used for randomness.
	int                 _parent;            //The id of our parent bot, if any.
	int                 _children;          //How many child robots we have spawned and are active.
	int                 _cooldown_charge;   //Next timestamp when we can lunge at the player.
	int                 _cooldown_melee;    //Next timestamp when we can hit again.
	int                 _cooldown_ouch;     //Next timestamp when we can play pain audio.
	int                 _cooldown_smoke;    //Next timestamp when we can emit smoke. (After death.)
	int									_cooldown_stun;			//Timestamp when we STOP being stunned.
	int									_cooldown_pain;			//Timestamp when we STOP being pained.
	int                 _last_proximity;		//Last timestamp when we beeped.
	int									_next_dependant_check;//When we need to look and make sure our parent is still alive.
	bool                _is_retired;        //Dead and waiting to be deleted.
	bool                _is_alerted;        //Has spotted the player
	bool                _is_dead;           //Hitpoints at zero, so we're just animating the corpse.
	bool                _is_boreing;        //True if we're tunneling through level geometry.
	bool                _is_burrowed;
	bool								_is_onscreen;				//True if the robot is within screen bounds. (Even if occluded.)
	bool                _is_hanging;
	bool                _is_falling;
	bool								_is_pained;					//True if we've been recently hit.
	bool                _is_near_wall;      //True if the robot is occupying a grid tile where WorldCellShape () isn't zero, and is thus near a wall.
	bool								_is_instakilled;		//True if the robot was killed in one shot.

	int                 _hitpoints;         //Duh.
	int                 _attack_number;     //Incremented at every attack.
	int                 _next_launcher;     //Which of our available missile launchers will be used next
	int                 _next_laser;        //Which of our available laser cannons will be used next
	int                 _time_death;        //Game tick when bot entered death state.
	int                 _xp;                //How much XP this bot is carrying.
	int                 _proximity_pulse;   //So the warning flash can last more than one frame. Counts down to when it turns off.

	float               _death_spin;        //How fast the body is spinning as it falls.
	GLvector2           _death_momentum;    //Movement of dead bot.
	GLvector2           _impact_kick;       //Cosmetic shoving of bot in respone to bullets.
	float               _angle;
	float               _death_fade;        //1.0 at death, diminishes per-frame.
	vector<bodyLeg>     _legs;
	vector<Weakpoint>   _weak_point;
	vector<SpriteUnit>  _wp_sprite;
	vector<Weapon>      _weapons;

	int                 _ai_state;
	int                 _ai_priorities[4];
	int                 _ai_cooldown;
	bool                _ai_flip;
	float               _ai_goal_distance;       //How far from our goal are we?
	float								_ai_player_distance;
	float               _ai_speed;

	GLvector2           _ai_move[MOVE_COUNT];

	/// These are our Ai Cores.
	void                AiPounce();
	void                AiBossReflector();
	void                AiBeeline();
	void                AiDeath();
	void                AiTunnel();
	void                AiSentry();
	void                AiOrbit();
	void                AiGuard();
	void                AiHitnrun();
	void                AiWalk();

	/// All of these are called are part of the normal update cycle.
	void                DoThink();
	bool                DoAttack();
	void                DoMove();
	void                DoFire();
	void                DoBody();
	void                DoBbox();
	void                DoProximity();

	//
	void                Alert();
	GLvector2						Bob(int cycle);
	void                Burrow();
	void                BuildBody();
	void                BuildLegs();
	void                Hang();
	void                Kick(GLvector2); //Cosmetically shove the robot in response to impacts.
	GLvector2           SoundOrigin();
	void                FaceTarget();
	void                FindFloor();
	void                FindCeiling();
	void								FindOpenSpot ();
	bool                TryMove(int dir, GLvector2* new_pos);
	bool                TryCollide(GLvector2 new_pos, bool avoid_walls = false);
	void                DropPowerups();
	bool                CanSeePlayer();
	void                MoveEye();
	void                PredictPlayer(const Projectile* p);
	int                 ShotRhythm();
	bool								WantToAttack();

public:

	//The "main" color of the robot.
	GLrgba							BodyColor () { return _body_color; }
	/// Deal non-directional damage (such as explosion blast) to the bot.
	/// Damage is halved unless critical is true.
	void                Damage(int damage);
	/// Deal directional damage to the bot. IF it results in death, the impact
	/// vector is used to animate the corpse.
	void                Damage(int damage, GLvector2 impact, GLvector2 direction);
	/// Kill the robot. This does NOT remove it from the game, it only causes the body to fall.
	/// Use Retire () to eliminate it entirely.
	void                Die();
	/// Returns TRUE if the bot has died and is not a corpse.
	bool                Dead() { return _is_dead; }
	/// Returns TRUE if the given impact position would result in a headshot.
	/// Returns TRUE if the given point is touching the bot.
	/// If take_damage is false on return, then don't send the bot a damage packet.
	bool                Hit(GLvector2 pos, bool& take_damage);
	///Returns true if the robot can only be harmed by shooting its weak points.
	bool                HasWeakpoints() { return _config->has_weakpoints; }
	//Id is used by parents and children to find each other in the heap.
	int                 Id() { return _id; }
	bool								Invulnerable () { return _config->is_invulnerable; }
	/// Initialize the bot as the given type, existing at the given location.
	/// Bot begins in the non-alerted state.
	void                Init(GLvector2 position, string type);
	void                Init(GLvector2 position, int type_index);
	/// Returns TRUE if the bot is actively trying to kill the player.
	bool                IsAlerted() { return _is_alerted; }
	/// Returns TRUE if this is a boss monster.
	bool                IsBoss() { return _config->is_boss; }
	/// Used to shove this robot in the given direction over many frames.
	void                Launch(GLvector2 direction);
	/// Returns the origin (center) of the HEAD.
	GLvector2           Position() { return _position; }
	//Tells this robot the id of its parent. Used when robots spawn other robots.
	void                ParentSet(int id) { _parent = id; }
	//When a child bot is retired, it calls this on its parent.
	void                ChildRetired();
	/// Returns true if the bot is awaiting deletion.
	bool                Retired() { return _is_retired; }
	/// Cause the bot to retire itself, which will flag it for deletion.
	void                Retire();
	//void                Render();

	void								RenderBody ();
	void								RenderDebug ();
	void								RenderEye ();
	void								RenderIris ();
	void								RenderPain ();

	/// Render the (!) icon in place of the robot. Used by the scanner pickup.
	void                RenderHidden();
	/// The RADIUS of the head.
	float               Size() { return _config->size; }
	/// Calling this will nudge the bot in the direction of the given vector.
	void                Shove(GLvector2 vector);
	/// Cause the bot to think, move, shoot, animate, etc.
	void                Update();
	/// Return how much XP the bot is carrying.
	int                 Xp() { return _xp; }
	/// Give the bot XP to carry around.
	void                XpSet(int value) { _xp = value; }
};

#endif // ROBOT_H
