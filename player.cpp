/*-----------------------------------------------------------------------------

  Player.cpp

  This handles the position, movement, and input of the player. Their
  visibility circle is handled in visible.cpp, the character is animated in
  avatar.cpp, and the player info is drawn in hud.cpp.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "avatar.h"
#include "camera.h"
#include "collision.h"
#include "entity.h"
#include "hud.h"
#include "input.h"
#include "InputManager.h"
#include "menu.h"
#include "page.h"
#include "particle.h"
#include "player.h"
#include "projectile.h"
#include "random.h"
#include "render.h"
#include "robot.h"
#include "system.h"
#include "transition.h"
#include "trivia.h"
#include "world.h"
#include "zone.h"

using namespace pyrodactyl;

#define RECHARGE_INTERVAL     600
#define PLAYER_SIZE           0.5f
#define SHIELD_INTERVAL       200
#define DEATH_FADE_TIME       3000
#define DEATH_MENU_WAIT       1500
#define GATHER_INTERVAL       666
//The size of the "Incoming Missile!" warning arrows.
#define WARNING_ARROW_SIZE    0.15f
#define ACCESS_DISTANCE       1.0f
#define GATHER_DISTANCE       1.0f
#define MESSAGE_OFFSETS       (sizeof (message_offset)/sizeof (float))
//Internal scaling. Use gameplay.ini to modify the in-game spread value.
#define BULLET_SPREAD         0.1f
//There's death of player AND death of avatar. I actually don't remember why. - SY
#define DEAD                  (player_dead || avatar.Dead ())
//How many milliseconds of invulnerability after getting hit.
#define GRACE_PERIOD          1000
//Size of the effect bubble around the player when hit.
#define SHIELD_EFFECT_SIZE    PLAYER_SIZE + 0.2f
//Begin emitting smoke when the player is below this portion of their health.
#define LOW_HEALTH            0.25
//How fast the player moves through doors.
#define AUTO_SPEED            0.03f
//How strongly does the hat tilt as we move.
#define HAT_TILT							500.0f

static float message_offset[] = { 0.5f, -0.25f, 0.25f, -0.5f, 0.0f, -0.75f };

struct Gather
{
	int     cooldown;
	int     count;
	bool    active;
};

static string							zone_list;

static GLvector2          aim;
static float              aim_angle;
static PlayerStats        stats;
static GLvector2          position;
static GLvector2          momentum;
static GLvector2          right_stick;
static Avatar             avatar;
static int                shield_cooldown;
static int                skate_sound_cooldown;
static bool               player_dead;
static float              current_speed;
//The value of the current fade-to-red after death.
static float              death_fade;
static int                tick_died;

static vector<GLvector2>  incoming;
static vector<GLvector2>  incoming_draw;
static int                warning_blink;
static float              cursor_glow;
static Gather             gathering_xp;
static float              distance_traveled;
static int                current_message_offset;
//This is used to offer the player "press a button to do a thing",
//like access a machine or use and object.
static bool               access_available;
static bool               access_activated;
static string             access_message;
static float              access_proximity;
//Grace period is the frames of invulnerability after respawning.
static int                grace_begin;
//Smoke cooldown is used when a damaged player needs to emit smoke.
static int                smoke_cooldown;
//Auto drive is when the game moves the player, as during a level change.
static bool               auto_drive;
static GLvector2          auto_direction;

static fxHat              my_hat;

static float              compass_angle;
static CompassState       compass_state;

static int								trivia_damage_this_frame;
static int								trivia_kills_this_frame;
static int								trivia_points_this_frame;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void do_compass()
{
	int             current_room;
	const Zone*     z = WorldZone();
	GLvector2       goal;
	float           desired_angle;
	float           adjust;

	current_room = z->RoomFromPosition(position);
	if (current_room > z->RoomCount() - 1) {
		compass_state = COMPASS_OFF;
		return;
	}

	if (current_room == z->RoomCount() - 1)
	{
		compass_state = COMPASS_EXIT;
		return;
	}

	goal = z->RoomPosition(current_room + 1);
	desired_angle = GLvector2(position - goal).Angle();
	adjust = syMathAngleDifference(desired_angle, compass_angle);
	compass_angle += adjust * 0.05f;
	compass_state = COMPASS_ON;
}

static void do_drop_hat(GLvector2 direction)
{
	fxHat*			h = new fxHat;
	GLvector2		movement;

	PlayerStats *p = Player();
	p->Hat(false);
	p->HatLevel(-1);
	p->HatZone(-1);
	p->HatLost(true);

	movement = direction * 0.05f;
	movement.y = min(movement.y, 0.0f);
	*h = my_hat;
	h->Drop(movement, (RandomFloat() - 0.5f) * 10.0f);
	EntityFxAdd(h);
}

static void teleport_particles(GLvector2 pos)
{
	GLrgba    color1, color2;

	color1 = color2 = Player()->Color();
	color2.Negative();
	for (int i = 0; i <= 20; i++) {
		float     v;
		float     size;

		v = (float)i / 20.0f;
		v = pow(v, 3) / 10.0f;
		size = (1.0f - v) / 10;
		ParticleGlow(pos, GLvector2(0, v), color1, color2, 2, size);
		ParticleGlow(pos, GLvector2(0, -v), color1, color2, 2, size);
	}
}

static void do_message(const char* message, ...)
{
	static char     msg_text[64];
	va_list         marker;
	fxMessage*      m;
	GLvector2       msg_pos;

	va_start(marker, message);
	vsprintf(msg_text, message, marker);
	va_end(marker);
	float y = message_offset[current_message_offset];
	current_message_offset = (current_message_offset + 1) % MESSAGE_OFFSETS;
	msg_pos = position + GLvector2(0, y);
	m = new fxMessage;
	m->Init(msg_pos, msg_text);
	EntityFxAdd(m);
}

static void do_cheat()
{
	if (EnvValueb(ENV_CHEATS)) {
		//For testing: Cycle all available primary weapons.
		if (InputKeyPressed (SDL_SCANCODE_Q)) {
			AudioPlay("missile_get");

			int id = EnvProjectileNext(stats.Weapon(PLAYER_WEAPON_PRIMARY)->Info()->_id, PROJECTILE_PRIMARY);
			stats.Weapon(PLAYER_WEAPON_PRIMARY)->Equip(EnvProjectileFromId(id));
			do_message(EnvProjectileFromId(id)->_title.c_str());
		}
		//Cycle through all available secondary weapons.
		if (InputKeyPressed (SDL_SCANCODE_E)) {
			AudioPlay("missile_load");

			int id = EnvProjectileNext(stats.Weapon(PLAYER_WEAPON_SECONDARY)->Info()->_id, PROJECTILE_SECONDARY);
			stats.Weapon(PLAYER_WEAPON_SECONDARY)->Equip(EnvProjectileFromId(id));
			do_message(EnvProjectileFromId(id)->_title.c_str());
		}
		//Open the store menu.
		if (InputKeyPressed(SDL_SCANCODE_M))
			MenuOpen(MENU_STORE);
		//Open upgrade menu
		if (InputKeyPressed (SDL_SCANCODE_N))
			MenuOpen(MENU_UPGRADE);
		//Open the hat store
		if (InputKeyPressed (SDL_SCANCODE_H))
			MenuOpen (MENU_HAT);
	}
}

static void do_access()
{
	string            message;

	access_available = false;
	if (InputJoystickActive()) {
		message = "Press {#ff0}A {###} ";
	}
	else {
		message = "Press {#ff0}";
		message += SDL_GetScancodeName(gInput.iv[CONTROL_ACTIVATE].key_val);
		message += " {###} ";
	}
	message += access_message;
	HudMessage(message);
}

static void do_sounds()
{
	float   speed;
	//float   proximity;

	speed = current_speed / EnvPlayerSpeed(EnvSkillMaxLevel(SKILL_SPEED)).val;
	AudioLoop(LOOP_MOVE, 0.5f + speed, speed / 4);
	//proximity = (access_proximity / ACCESS_DISTANCE) * 0.5f;
	//AudioLoop (LOOP_ACCESS, 1.0f, proximity);
}

static void do_incoming()
{
	float       pitch_number;
	float       pitch_proximity;
	float       pitch_final;
	float       distance;
	float       nearest;
	GLvector2   offset;
	GLrgba      color;
	SpriteBox   box;

	if (incoming.empty()) {
		incoming_draw.clear();
		AudioLoop(LOOP_INCOMING, 1.0f, 0.0f);
		return;
	}
	warning_blink = (warning_blink + 1) % 2;
	if (warning_blink)
		color = GLrgba(1, 0, 0);
	else
		color = GLrgba(1, 1, 0);

	//Look at incoming missiles and see which ones need directional indicators.
	nearest = 999999.0f;
	GLvector camera_current = CameraPosition();
	for (unsigned i = 0; i < incoming.size(); i++) {
		offset = incoming[i] - position;
		distance = offset.Length();
		if (distance > camera_current.z) {
			offset.Normalize();
			box.Init(PlayerPosition() + offset, WARNING_ARROW_SIZE, offset.Angle(), SPRITE_UI, SPRITE_UI_WARNING);
			box.ColorSet(color);
			HudAddSprite(box);
		}
		nearest = min(nearest, distance);
	}
	//Look at the number and proximity of incoming missiles to determine
	//how intense the warning sound needs to be.
	nearest = 1.0f - (min(nearest, 10.0f) / 10.0f);
	pitch_proximity = nearest * 5.0f;
	pitch_number = (float)incoming.size() * 0.2f;
	pitch_final = 0.3f + max(pitch_number, pitch_proximity);
	AudioLoop(LOOP_INCOMING, "alarm", pitch_final, 0.4f);
	incoming.clear();
}

static GLvector2 do_collision(GLvector2 position, GLvector2* movement_in)
{
	GLvector    surface3D;
	GLvector    movement3D;
	GLvector    new_movement;
	GLvector2   wall;
	GLvector2   new_pos;
	GLvector2   movement;
	float       depth;
	float       dot;

	if (EnvValueb(ENV_NOCLIP))
		return position + *movement_in;
	movement = *movement_in;
	//Make sure we don't move too fast.
	current_speed = movement.Length();
	*movement_in = movement;
	new_pos = position + movement;
	if (!Collision(new_pos, &wall, &depth, 0.12f)) {
		return new_pos;
	}
	//Set up some 3D vectors for reflection.
	surface3D = GLvector(wall.x, wall.y, 0.0f).Normalized();
	movement3D = GLvector(movement.x, movement.y, 0).Normalized();
	dot = GLdot(movement3D, surface3D);
	//Only bounce if we're running INTO the wall, not away. Otherwise, glitching
	//slightly into the surface for one frame would suck us INTO the wall.
	if (dot < 0) {
		//if we have the powerup, slide along the wall.
		/*if (stats.Ability(ABILITY_GLIDE)) {
			ParticleGlow(new_pos, movement * -1, stats.Color(), stats.Color(), 1, 0.1f);
			//We nudge the actor out of the wall before we try to bounce off of it.
			new_pos += wall * depth * 1.1f;
			new_movement = GLreflect2(movement3D, surface3D).Normalized();
			new_movement = CollisionSlide(wall, GLvector2(new_movement.x, new_movement.y));
			movement = GLvector2(new_movement.x, new_movement.y);
			movement.Normalize();
			movement *= current_speed;
			if (GameTick() > skate_sound_cooldown) {
			skate_sound_cooldown = GameTick() + 1000;
			AudioPlay(SOUND_SKATE);
			}
			}
			else*/
		{ //bounce off the wall
			float bounce_velocity = Env().bounce_velocity;
			int   damage = WorldZone()->WallDamage();

			if (damage > 0) { //If walls are electrified.
				bounce_velocity = 0.98f;
				AudioPlay("forcefield_bounce.wav", 1.0f);
				ParticleSparks(position, WorldZone()->Color(COLOR_FOREGROUND), 5);
				PlayerDamage(damage);
			}
			else { //Just bounce off normally
				ParticleRubble(PlayerPosition(), 0.08f, 4);
				AudioPlay("collide");
			}

			//We nudge the actor out of the wall before we try to bounce off of it.
			new_pos += wall * depth * 1.1f;
			new_movement = GLreflect2(movement3D, surface3D).Normalized();
			movement = GLvector2(new_movement.x, new_movement.y);
			movement.Normalize();
			movement *= current_speed * bounce_velocity;
		}
	}
	new_pos += movement;
	*movement_in = movement;
	return new_pos;
}

static void do_fire()
{
	if (DEAD)
		return;

	if (EnvValueb(ENV_CONTROLLER_ALT)) 	{
		stats.Weapon(PLAYER_WEAPON_PRIMARY)->Update(avatar.OriginLaser(), gInput.State(CONTROL_PRIMARY));
	}	else {
		stats.Weapon(PLAYER_WEAPON_PRIMARY)->Update(avatar.OriginLaser(), gInput.State(CONTROL_PRIMARY) || gInput.RightAxisFire());
	}
	stats.Weapon(PLAYER_WEAPON_SECONDARY)->Update(avatar.OriginLauncher(), gInput.State(CONTROL_SECONDARY));
	if (stats.Weapon (PLAYER_WEAPON_SECONDARY)->Fired ()) 
		SystemRumble (1.0f, 200);
}

static GLvector2 do_movement()
{
	GLvector2   movement;
	GLcoord2    mouse;
	bool        aim_changed;

	movement = GLvector2();
	aim_changed = false;
	if (DEAD)
		return movement;
	if (!InputJoystickActive()) {
		if (gInput.State(CONTROL_UP))
			movement += GLvector2(0, -1);
		if (gInput.State(CONTROL_DOWN))
			movement += GLvector2(0, 1);
		if (gInput.State(CONTROL_LEFT))
			movement += GLvector2(-1, 0);
		if (gInput.State(CONTROL_RIGHT))
			movement += GLvector2(1, 0);
		//Normalize so diagonal isn't faster than single-axis movement.
		movement.Normalize();
		mouse = InputMouseMovement();
		if (mouse.x || mouse.y) {
			aim += GLvector2((float)mouse.y, (float)mouse.x) * EnvValuef(ENV_MOUSE_SPEED) * 0.01f;
			aim_changed = true;
		}
	}	else { //using joystick
		if (InputAxisNeutral(JOY_LEFT_STICK_X) && InputAxisNeutral(JOY_LEFT_STICK_Y)) {
			movement = GLvector2();
		}
		else { //One or more axis is out of the dead zone, so use both.
			movement.x = InputAxisf(JOY_LEFT_STICK_X);
			movement.y = InputAxisf(JOY_LEFT_STICK_Y);
		}
		if (InputAxisNeutral(JOY_RIGHT_STICK_X) && InputAxisNeutral(JOY_RIGHT_STICK_Y)) {
		}
		else { //One or more axis is out of the dead zone, so use both.
			right_stick.x = (float)InputAxisf(JOY_RIGHT_STICK_X);
			right_stick.y = (float)InputAxisf(JOY_RIGHT_STICK_Y);
			aim_changed = true;
		}
		right_stick.Normalize();
		aim = position + right_stick;
	}
	if (movement.Length() > 1.1f) {
		float f = movement.Length();
		f += 0.0f;
		movement.Normalize();
	}

	movement *= EnvPlayerSpeed(stats.Skill(SKILL_SPEED)).val * Env().acceleration;
	if (aim_changed)
		cursor_glow = min(cursor_glow + 0.1f, 3.0f);
	else
		cursor_glow *= 0.95f;
	//Move our cursor with us.
	aim += CameraMoved();
	//Keep the cursor within the screen bounds.
	GLvector  camera_current = CameraPosition();
	GLvector  limit;

	limit.y = camera_current.z;
	limit.x = limit.y * RenderAspect();
	aim.x = clamp(aim.x, camera_current.x - limit.x, camera_current.x + limit.x);
	aim.y = clamp(aim.y, camera_current.y - limit.y, camera_current.y + limit.y);
	aim_angle = (position - aim).Angle();
	return movement;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

float PlayerDeathFade() { return death_fade; }

void PlayerAutoDrive(GLvector2 direction)
{
	auto_drive = true;
	auto_direction = direction;
}

void PlayerOfferAccess(float distance, const char* message)
{
	if (distance < ACCESS_DISTANCE) {
		access_message = message;
		access_available = true;
		access_proximity = max(access_proximity, ACCESS_DISTANCE - distance);
	}
}

bool PlayerAccessActivated()
{
	return access_activated;
}

void PlayerIncomingMissile(GLvector2 position)
{
	incoming.push_back(position);
}

float PlayerGatherDistance()
{
	if (stats.Ability(ABILITY_MAGNET))
		return 15.0f; //Grab almost everything we can see
	else
		return GATHER_DISTANCE;
}

void PlayerAddXp(int coins)
{
	gathering_xp.count += coins;
	gathering_xp.active = true;
	gathering_xp.cooldown = GameTick() + GATHER_INTERVAL;
}

void PlayerDealDamage(int damage)
{
	trivia_damage_this_frame += damage;
}

void PlayerScoreKill()
{
	trivia_kills_this_frame++;
}

void PlayerScorePoints(int pts)
{
	trivia_points_this_frame += pts;
}

bool PlayerCollide(GLvector2 hit)
{
	GLvector2     offset;

	offset = hit - position;
	if (abs(offset.x) > PLAYER_SIZE || abs(offset.y) > PLAYER_SIZE)
		return false;
	return avatar.Collide(hit);
}

int PlayerShieldChance()
{
	int   chance;
	float scale;

	//Returns 1 in n chance of getting a shield boost. This is scaled so that
	//they get more common as health gets lower.
	scale = (float)stats.Shields() / (float)stats.ShieldsMaxValue();
	scale *= 10;
	chance = clamp((int)scale, 2, 10);
	return chance;
}

void PlayerShove(GLvector2 force)
{
	if (!PlayerIgnore())
		momentum += force;
}

void PlayerDamage(int points)
{
	PlayerDamage(points, GLvector2(0, 1));
}

void PlayerDamage(int points, GLvector2 direction)
{
	int           angle;
	fxShieldHit*  s;
	GLrgba        flash_color;
	float         intensity;

	if (PlayerIgnore())
		return;
	if (Player()->Hat()) {
		do_drop_hat(direction);
		points = 0;
	}
	AudioPlay("player_hit");
	//Scale the flash intensity based on the percent of our health it took.
	//Scale it so getting hit for 1/5 of HP is max intensity.
	if (points) {
		intensity = ((float)points / (float)stats.ShieldsMaxValue()) * 5;
		intensity = clamp(intensity, 0.25f, 1.0f);
		SystemRumble (intensity, 250);
		HudFlash(GLrgba(1, 0.1f, 0.1f)*intensity);
		CameraShake(Env().screen_shake_player_dmg * intensity);

		if (EnvValueb(ENV_NODAMAGE))
			return;
		stats.TriviaModify(TRIVIA_DAMAGE_TAKEN, points);
		stats.ShieldsModify(-points);
		if (stats.Dead())
			tick_died = GameTick();
	}
	if (GameTick() < shield_cooldown)
		return;
	shield_cooldown = GameTick() + SHIELD_INTERVAL;
	angle = 360 - (int)syMathAngle(0, 0, direction.x, direction.y);
	s = new fxShieldHit;
	s->Init(GLrgba(1, 0.3f, 0.3f), GameTick(), SHIELD_EFFECT_SIZE);
	s->HitAngle(angle);
	EntityFxAdd(s);
}

void PlayerRespawn(GLvector2 pos)
{
	player_dead = false;
	auto_drive = false;
	stats.RestoreShields();
	position = pos;
	momentum = GLvector2();
	right_stick = GLvector2(1, 0);
	aim = GLvector2(position.x, position.y) + GLvector2(0, 1);
	avatar.Spawn(position, stats, false);
	//camera_current = GLvector (pos.x, pos.y, CAMERA_DISTANCE);
	aim = pos;
	tick_died = 0;
}

const char* PlayerZones() { return zone_list.c_str(); }

void PlayerHatInit()
{
	PlayerStats *p = Player();
	my_hat.Init(SpriteEntryLookup(p->HatSprite()), p->HatSize(), p->HatColor());
}

void PlayerHatGive(string name, string sprite_name, float size, GLrgba color)
{
	PlayerStats *p = Player();

	//If we're already wearing a hat, drop it BUT don't update the level and zone we got it in
	//This is so players can switch hats in shops without ruining their achievement progress
	if (p->Hat())
		do_drop_hat(GLvector2());
	else {
		//Set the level and zone of hat purchased to current values
		p->HatLevel(WorldLevelIndex());
		p->HatZone(WorldZoneIndex());
	}

	p->Hat(true);
	p->HatName(name);
	p->HatSprite(sprite_name);
	p->HatSize(size);
	p->HatColor(color);
	p->HatPurchaseCountInc();

	my_hat.Init(SpriteEntryLookup(sprite_name), size, color);
}

void PlayerSpawn(GLvector2 pos)
{
	player_dead = false;
	auto_drive = false;
	tick_died = 0;
	position = pos;
	momentum = GLvector2();
	right_stick = GLvector2(1, 0);
	aim = GLvector2(position.x, position.y) + GLvector2(0, 1);
	avatar.Init(stats.Character());
	avatar.Spawn(position, stats, true);
	CameraInit(GLvector(pos.x, pos.y, stats.VisionRadius()));
	aim = pos;
	gathering_xp.count = 0;
	gathering_xp.active = false;
	gathering_xp.cooldown = 0;
	if (stats.Dead())
		PlayerRespawn(pos);

	int   zones = WorldMap()->ZoneCount();

	zone_list = "";
	for (int i = 0; i < zones; i++) {
		if (WorldPlayerLocation().y == i) //If this is our current zone.
			zone_list += StringSprintf("{%d} ", i);
		else if (stats.ZoneComplete(WorldPlayerLocation().x, i))//if the zone is complete:
			zone_list += StringSprintf("[%d] ", i);
		else
			zone_list += StringSprintf("(%d) ", i);
	}
	if (stats.Hat())
		my_hat.Init(SpriteEntryLookup(stats.HatSprite()), stats.HatSize(), stats.HatColor());
}

GLvector2 PlayerPosition()
{
	if (DEAD)
		return position;
	return avatar.Center();
}

GLvector2 PlayerOrigin()
{
	return position;
}

void PlayerOriginSet(GLvector2 new_pos)
{
	position = new_pos;
}

GLvector2     PlayerHead() { return avatar.OriginHead(); }
GLvector2     PlayerMomentum() { return momentum; }
GLvector2     PlayerAim() { return aim; }
PlayerStats*  Player() { return &stats; }

//Technically not used, but left here in case we want to add characters late.
float PlayerSize()
{
	return PLAYER_SIZE;
}

//If the player is dead or otherwise unavailable for killing.
bool PlayerIgnore()
{
	if (player_dead)
		return true;
	if (avatar.Dead())
		return true;
	if (TransitionActive())
		return true;
	if (GameTick() - grace_begin < GRACE_PERIOD)
		return true;
	return false;
}

void PlayerUpdate()
{
	GLcoord2  mouse;
	GLcoord2  shoot_vector;
	GLvector2 movement;
	GLvector  head;
	GLvector2 last_position;
	GLvector2 new_pos;
	GLbbox    constrain;
	int       now;

	if (!GameActive())
		return;
	now = GameTick();
	//Calling steam stats many times a frame is RIDICULOUSLY expensive. So, we queue
	//up all the changes for the whole frame and apply them all at once.
	if (trivia_damage_this_frame) {
		Player()->TriviaModify(TRIVIA_DAMAGE_DEALT, trivia_damage_this_frame);
		trivia_damage_this_frame = 0;
	}
	if (trivia_kills_this_frame) {
		Player()->EventRobotsKill(trivia_kills_this_frame);
		trivia_kills_this_frame = 0;
	}
	if (trivia_points_this_frame) {
		Player()->ScorePoints(trivia_points_this_frame);
		trivia_points_this_frame = 0;
	}

	death_fade = 0.0f;
	if (player_dead) {
		death_fade = (float)(GameTick() - tick_died) / DEATH_FADE_TIME;
		death_fade = clamp(death_fade, 0.0f, 1.0f);
	}
	if (DEAD)
		grace_begin = now;
	do_compass();
	stats.Update();
	//If the player moves out of the world bounds, shove them back in.
	//Mostly used to push them back down when they fly into the sky in city levels.
	if (!auto_drive) {
		GLbbox2 world = WorldBounds();
		if (position.y < world.pmin.y)
			PlayerShove(GLvector2(0, world.pmin.y - position.y) * 0.01f);
		if (position.x < world.pmin.x)
			PlayerShove(GLvector2(world.pmin.x - position.x, 0) * 0.01f);
		do_incoming();
		stats.TriviaModify(TRIVIA_PLAYTIME, UPDATE_INTERVAL);
	}

	//We're dead, oops
	if (player_dead) {
		bool    reset = gInput.Pressed(CONTROL_ACTIVATE);

		//If the player doesn't push any buttons, then eventually open the menu anyway.
		if ((GameTick() - tick_died) > DEATH_FADE_TIME)
			reset = true;
		//If the player pushes buttons right away, ignore them for a few seconds
		//so they can see what killed them.
		if ((GameTick() - tick_died) < DEATH_MENU_WAIT)
			reset = false;
		//No warranty, take them to the game over screen
		if (!stats.Warranty() && !EnvValueb(ENV_CHEATS) && reset) {
			GameEnd();
			MenuOpen(MENU_GAMEOVER);
			stats.SaveHighScore();
		}
		//Does the player possess a warranty?
		if ((stats.Warranty() || EnvValueb(ENV_CHEATS)) && reset) {
			//Yes, respawn at the nearest checkpoint when the press the activate button
			PlayerRespawn(WorldLanding(stats.CheckpointGet()));
			//Warranty is used up now
			stats.WarrantySet(false);
		}
	}
	if (gInput.State(CONTROL_ACTIVATE))
		TriviaAdvance();
	if (gathering_xp.active && GameTick() > gathering_xp.cooldown) {
		AudioPlay("coin", position);
		stats.MoneyGive(gathering_xp.count);
		do_message("${#ff0}%d{###}", gathering_xp.count);
		gathering_xp.active = false;
		gathering_xp.count = 0;
	}
	if (EnvValueb(ENV_NODIE) && stats.Shields() < 1) {
		stats.ShieldsSet(1);
	}
	//See if we just died.
	if (stats.Shields() < 1 && !player_dead) {
		fxExplosion*  e;

		e = new fxExplosion;
		e->Init(OWNER_ROBOTS, position, 0, PlayerSize());
		EntityFxAdd(e);
		avatar.Kill();
		stats.Kill(position, momentum);
		AudioPlay("player_death");
		ParticleDebris(PlayerPosition(), PLAYER_SIZE / 3, 15);
		player_dead = true;
	}
	//See if we need to emit smoke based on low health.
	if (stats.ShieldsPercent() < LOW_HEALTH && !player_dead && now > smoke_cooldown) {
		int cooldown;

		//The lower our health, the faster we emit smoke.
		cooldown = 150 + stats.Shields() * 3;
		cooldown = max(cooldown, 20);
		smoke_cooldown = now + cooldown;
		ParticleSmoke(position, avatar.Size()*(RandomFloat() + 1.0f), 2);
	}
	last_position = position;
	//if (EnvValueb(ENV_CHEATS))
	do_cheat();
	do_fire();
	access_activated = false;
	access_proximity *= 0.95f;
	if (gInput.State(CONTROL_ACTIVATE))
		access_activated = true;
	if (!auto_drive) {
		movement = GLvector2();
		if (!avatar.Dead())
			movement = do_movement();
		movement += momentum;
		current_speed = movement.Length();
		position = do_collision(position, &movement);
		distance_traveled += movement.Length();
		while (distance_traveled > ONE_METER) {
			stats.TriviaModify(TRIVIA_CM_TRAVELED, 100);
			distance_traveled -= ONE_METER;
		}
	}
	else {//auto-drive is active. (Pull player through doors.)
		position += auto_direction * AUTO_SPEED;
	}
	do_sounds();
	momentum = movement * Env().momentum_loss;
	avatar.Move(position, momentum, aim - position, aim_angle, &stats);
	if (Player()->Hat()) {
		float		angle = avatar.Momentum().x * -HAT_TILT;
		angle = clamp(angle, -20.0f, 20.0f);
		my_hat.Wear(avatar.OriginHead() + GLvector2(0.0f, -0.1f), angle);
	}
	//If we've been damaged recently, partly close eyes.
	if (GameTick() < shield_cooldown)
		avatar.Blink(0.66f);
	else
		avatar.Blink(0);
}

void PlayerRender()
{
	if (TransitionActive())
		return;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	avatar.Render();
	if (Player()->Hat())
		my_hat.Render();
	if (stats.Weapon(PLAYER_WEAPON_PRIMARY)->Firing())
		RenderQuad(avatar.OriginLaser(), SPRITE_GLOW, stats.Weapon(PLAYER_WEAPON_PRIMARY)->Color(), 0.3f, 0, DEPTH_UNITS, true);
	if (stats.Weapon(PLAYER_WEAPON_SECONDARY)->Firing())
		RenderQuad(avatar.OriginLauncher(), SPRITE_GLOW, stats.Weapon(PLAYER_WEAPON_SECONDARY)->Color(), 0.3f, 0, DEPTH_UNITS, true);
	//Render cooldown aura, for testing...
	if (0) {
		float   aura = (1.0f - stats.Weapon(PLAYER_WEAPON_SECONDARY)->Cooldown());
		aura = pow(aura, 4);
		RenderQuad(position, SPRITE_GLOW, stats.Weapon(PLAYER_WEAPON_SECONDARY)->Color() * aura, aura, 0, DEPTH_UNITS - 0.1f, true);
	}
	if (avatar.Dead())
		return;
	glDepthMask(false);
	glBlendFunc(GL_ONE, GL_ONE);
	GLrgba weapon_color;
	weapon_color = stats.Weapon(PLAYER_WEAPON_SECONDARY)->Color();
	glColor3fv(&weapon_color.red);
	RenderCircularBar(180, 180 + (int)360.0f * stats.Weapon(PLAYER_WEAPON_SECONDARY)->Cooldown(), PlayerSize(), PlayerPosition(), SPRITE_SHOCKWAVE);
	glDepthMask(true);
	if (access_available)
		do_access();
	if (Player()->Ability(ABILITY_TARGET_LASER)) {
		GLvector2     pt;
		GLvector2     start;
		GLvector2     end;
		GLvector2     side;
		GLvector2     vec;
		GLuvFrame*    uv;
		GLrgba        color;

		glBlendFunc(GL_ONE, GL_ONE);
		glDepthMask(false);
		color = GLrgba(1, 0, 0.3f);
		start = avatar.OriginLaser();
		vec = aim - avatar.OriginLaser();
		vec.Normalize();
		side = vec.TurnedRight();
		side *= 0.05f;
		end = avatar.OriginLaser() + vec * 16;
		uv = SpriteMapLookup(SPRITE_BEAM);
		glColor3fv(&color.red);
		glBegin(GL_QUADS);
		glTexCoord2fv(&uv->uv[1].x);
		glVertex3f(start.x - side.x, start.y - side.y, 0);
		glTexCoord2fv(&uv->uv[2].x);
		glVertex3f(start.x + side.x, start.y + side.y, 0);
		glTexCoord2fv(&uv->uv[3].x);
		glVertex3f(end.x + side.x, end.y + side.y, 0);
		glTexCoord2fv(&uv->uv[0].x);
		glVertex3f(end.x - side.x, end.y - side.y, 0);
		glEnd();
		glDepthMask(true);
	}
}

void PlayerInit()
{
}

void PlayerReload()
{
	avatar.Init(stats.Character());
	avatar.Spawn(position, stats, true);
}

CompassState PlayerCompass(float& angle)
{
	angle = compass_angle;
	return compass_state;
}