/*-----------------------------------------------------------------------------

  Robot.cpp

  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "bodyparts.h"
#include "camera.h"
#include "collision.h"
#include "drop.h"
#include "entity.h"
#include "env.h"
#include "fx.h"
#include "game.h"
#include "particle.h"
#include "player.h"
#include "projectile.h"
#include "random.h"
#include "render.h"
#include "robot.h"
#include "page.h"
#include "sprite.h"
#include "world.h"

#define SHOW_LAUNCHERS    0
#define SHOW_PREDICTED    0
#define SHOW_CELL         0
#define RHYTHM_COUNT      (sizeof (shot_rhythm) / sizeof (int))
#define SHOVE_POWER       0.015f
#define DEATH_SPIN        0.1f
#define OUCH_INTERVAL     2500
#define KICK_STRENGTH     0.25f
#define DEBRIS_SIZE       0.85f
#define MELEE_REACH       1.15f

#define LAUNCH_VELOCITY   0.07f
#define INERTIA_LOSS      0.99f
#define BOB_CYCLE					3600
#define PAIN_TIME					500

static int                shot_rhythm[] =
{ 100, 360, 180, 240, 150, 150, 150, 220, 90, 280, 110, 330, 120, 180, 200, 140 };

static int                seed;
static GLvector2          ordinal_direction[8];
static bool               init;
static int                rhythm_index;
static SpriteEntry				sprite_square;
static float							bob[BOB_CYCLE];

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void do_init()
{
	GLvector2   corners[4];

	init = true;
	//These are used by robots attempting to wander aimlessly
	ordinal_direction[0] = GLvector2(0, -1); //N
	ordinal_direction[1] = GLvector2(1, -1); //NE
	ordinal_direction[2] = GLvector2(1, 0); //E
	ordinal_direction[3] = GLvector2(1, 1); //SE
	ordinal_direction[4] = GLvector2(0, 1); //S
	ordinal_direction[5] = GLvector2(-1, 1); //SW
	ordinal_direction[6] = GLvector2(-1, 0); //W
	ordinal_direction[7] = GLvector2(-1, -1); //NW
	for (int i = 0; i < 8; i++)
		ordinal_direction[i].Normalize();
	for (int i = 0; i < BOB_CYCLE; i++) {
		float angle = ((float)i / BOB_CYCLE) * 360.0f;
		bob[i] = sin(angle * DEGREES_TO_RADIANS) * 0.1f;
	}
	sprite_square = SpriteEntryLookup("Square");
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

GLvector2 Robot::Bob(int cycle)
{
	//Only single-sprite bots should bob in place.
	if (_body_part_count < 2)
		return GLvector2();
	cycle %= BOB_CYCLE;
	if (cycle < 0)
		cycle += BOB_CYCLE;
	return GLvector2(0, bob[cycle]);
}

int Robot::ShotRhythm()
{
	rhythm_index = (rhythm_index + 1) % RHYTHM_COUNT;
	return shot_rhythm[rhythm_index];
}

//Starting from out current location, try to move to a place where we
//aren't colliding with level geometry.
void Robot::FindOpenSpot()
{
	GLvector2		new_pos;
	int					sanity = 4;

	new_pos = _position;
	while (TryCollide(_position)) {
		GLvector2   normal;
		float				depth;

		//Find out what wall we're hitting and get its normal.
		Collision(new_pos, &normal, &depth, _config->size);
		//Now move out of the wall.
		new_pos += normal.Normalized() * depth * 1.1f;
		sanity--;
		//If this takes too many passes, then it means we're wedged in a spot where
		//we don't fit and we'll never get out like this. Rather than push the robot
		//totally out of the level, just leave them where they spawned and hope the
		//player can at least shoot it.
		if (sanity == 0)
			return;
	}
	_position = new_pos;
}

void Robot::FindFloor()
{
	GLcoord2    test;
	GLvector2   new_pos;
	int         steps;

	test = GLcoord2((int)_position.x, (int)_position.y);
	steps = 0;
	while (steps < 30) {
		if (WorldCellShape(test)) {
			test.y--;
			break;
		}
		test.y++;
	}
	new_pos = test;
	_position.y = CollisionFloor(new_pos);
	_position.y -= _config->size / 2;
}

void Robot::FindCeiling()
{
	GLcoord2    test;
	GLvector2   new_pos;
	int         steps;

	test = GLcoord2((int)_position.x, (int)_position.y + 1);
	steps = 0;
	while (steps < 30) {
		if (WorldCellShape(test)) {
			test.y++;
			break;
		}
		test.y--;
		steps++;
	}
	new_pos.x = _position.x;
	new_pos.y = (float)test.y;
	_position.y = CollisionCeiling(new_pos);
	_position.y += _config->size / 2;
}

void Robot::Burrow()
{
	FindFloor();
	_position.y += _config->size / 2;
	_is_burrowed = true;
	for (int i = 0; i < _body_part_count; i++) {
		_anchor[i] = _position + GLvector2(0, _config->size * (float)i);
		_sprite[i].Move(_anchor[i]);
	}
	DoBbox();
}

void Robot::Hang()
{
	FindCeiling();
	_is_hanging = true;
	for (int i = 0; i < _body_part_count; i++) {
		_anchor[i] = _position + GLvector2(0, _config->size * (float)((_body_part_count - 2) - i) * 0.5f);
		_sprite[i].Move(_anchor[i]);
	}
	_position = _sprite[0].Position();
	DoBbox();
}

void Robot::BuildLegs()
{
	bodyLeg   l;
	int       legs;
	int       legs_per_side;
	float     angle_step;
	float     angle_half;
	float     leg_length;
	float     knee_size;
	float     knee_height;
	float     step_size;
	bool      odd;

	legs = _config->legs;
	knee_size = _config->size * 0.4f;
	step_size = _config->size * 1.5f;
	odd = false;
	if (legs % 2) {
		odd = true;
		legs--;
	}
	legs_per_side = legs / 2;
	leg_length = _config->size * _config->walk_stride;
	knee_height = _config->size * _config->walk_height;
	if (legs_per_side) {
		angle_step = 360.0f / (float)legs_per_side;
		angle_half = angle_step / 2;
		for (int i = 0; i < legs_per_side; i++) {
			l.Init(_body_color, leg_length, knee_height, knee_size, step_size, angle_step * (float)i);
			_legs.push_back(l);
			l.Init(_body_color, -leg_length, knee_height, knee_size, step_size, angle_step * (float)i + angle_half);
			_legs.push_back(l);
			leg_length += leg_length;
		}
	}
	else { //What, ONE leg?!?!
		angle_step = 360.0f;
		angle_half = angle_step / 2;
	}
	if (odd) {
		l.Init(_body_color, 0, knee_height, knee_size, step_size * 2, angle_half / 2);
		_legs.push_back(l);
	}
	if (_config->is_boss) {
		for (unsigned i = 0; i < _legs.size(); i++)
			_legs[i].SoundStepSet("collide");
	}
}

void Robot::BuildBody()
{
	for (unsigned i = 0; i < MAX_PARTS; i++)
		_sprite[i].Init();
	if (_config->body == RBODY_FIXED) {
		_body_part_count = 1;
		_sprite[0].Init(_config->parts[0], _config->size, _body_color, _config->eye, _config->iris, _config->eye_color, _config->eye_size, _config->eye_offset);
	}
	else if (_config->body == RBODY_SNAKE || _config->body == RBODY_WORM) {
		_body_part_count = 1;
		_sprite[0].Init(_config->parts[0], _config->size, _body_color, _config->eye, _config->iris, _config->eye_color, _config->eye_size, _config->eye_offset);
		for (int i = 1; i < MAX_PARTS; i++) {
			if (_config->parts[i] == SPRITE_INVALID)
				break;
			_sprite[i].Init(_config->parts[i], _config->size*0.75f, _body_color);
			_body_part_count++;
		}
	}
	else if (_config->body == RBODY_SQUID) {
		_body_part_count = 1;
		_sprite[0].Init(_config->parts[0], _config->size, _body_color, _config->eye, _config->iris, _config->eye_color, _config->eye_size, _config->eye_offset);
		for (int i = 1; i < MAX_PARTS; i++) {
			if (_config->parts[i] == -1)
				break;
			_sprite[i].Init(_config->parts[i], _config->size, _body_color);
			_anchor[i] = _position;
			_body_part_count++;
		}
	}
	else if (_config->body == RBODY_JELLY) {
		_body_part_count = 2;
		_sprite[0].Init(_config->parts[0], _config->size, _body_color, _config->eye, _config->iris, _config->eye_color, _config->eye_size, _config->eye_offset);
		_sprite[1].Init(_config->parts[1], _config->size, _body_color);
	}
	else if (_config->body == RBODY_TURRET) {
		_body_part_count = 1;
		_sprite[0].Init(_config->parts[0], _config->size, _body_color, _config->eye, _config->iris, _config->eye_color, _config->eye_size, _config->eye_offset);
		for (int i = 1; i < 3; i++) {
			if (_config->parts[i] == -1)
				break;
			_sprite[i].Init(_config->parts[i], _config->size*0.5f, _body_color);
			_body_part_count++;
		}
	}
	for (int i = 0; i < _body_part_count; i++)
		_sprite[i].Move(_position, _angle);
	for (int i = 0; i < _body_part_count; i++)
		_sprite[i].SetColor(GLrgba());
	for (unsigned i = 0; i < _legs.size(); i++)
		_legs[i].ColorSet(GLrgba());
	_sprite[0].EyeEnable(false);
	//Build the weak points, if any.
	_wp_sprite.clear();
	for (unsigned i = 0; i < _config->weakpoint.size(); i++) {
		SpriteUnit      su;
		Weakpoint       w = _config->weakpoint[i];

		su.Init(w.sprite, w.size, w.color);
		_wp_sprite.push_back(su);
	}
}

void Robot::Retire()
{
	Robot*  bot;

	_is_retired = true;
	//Tell our parent we're gone.
	if (_parent && (bot = EntityRobotFromId(_parent)) != NULL)
		bot->ChildRetired();
}

void Robot::ChildRetired()
{
	_children--;
	if (_config->child_death_damage > 0)
		Damage(_config->child_death_damage);
}

void Robot::Init(GLvector2 position, RobotType type)
{
	Init(position, EnvRobotNameFromIndex(type));
}

void Robot::Init(GLvector2 position, string type)
{
	int     robot_type_number;

	if (!init)
		do_init();
	_type = type;
	robot_type_number = EnvRobotIndexFromName(type);
	if (robot_type_number == ROBOT_INVALID) {//instead of crashing game, just die
		_config = NULL;
		Retire();
		return;
	}
	//Get our unique id.
	_id = ++seed;
	_parent = 0;
	_children = 0;
	_bob = GLvector2();
	_proximity_pulse = 0;
	//First load the properties of this robot type
	_config = EnvRobotConfig(_type);
	_body_color = _config->body_color;
	_hitpoints = _config->hitpoints;
	_cooldown_charge = 0;
	_cooldown_melee = 0;
	_cooldown_ouch = 0;
	_cooldown_stun = 0;
	_cooldown_pain = 0;
	_cooldown_smoke = 0;
	_pain_sprite = 0;
	_ai_cooldown = 0;
	_attack_number = 0;
	_next_dependant_check = 0;
	_launch_inertia = GLvector2();
	_inertia = GLvector2();
	//We nudge our original position by a random ammount. This is so that we don't end up EXACTLY
	//on top of another robot, since then we wouldn't be able to shove each other free.
	_position = position + GLvector2(RandomFloat() * 0.01f, RandomFloat() * 0.01f);
	_ai_flip = (_id % 2) == 0;
	_angle = 0;
	_impact_kick = GLvector2();
	_death_spin = 0;
	_next_launcher = 0;
	_next_laser = 0;
	_is_retired = false;
	_is_alerted = false;
	_is_dead = false;
	_is_boreing = false;
	_is_burrowed = false;
	_is_hanging = false;
	_is_pained = false;
	_is_instakilled = false;
	_ai_state = AI_IDLE;
	_ai_speed = 0;
	_xp = _config->xp_value;
	_body_part_count = 0;
	_exhaust_point = GLvector2();
	_bore_point = _position;
	_legs.clear();
	if (_config->legs) {
		FindFloor();
		BuildLegs();
	}
	BuildBody();
	if (_config->ai_idle == AI_IDLE_BURROW)
		Burrow();
	if (_config->ai_idle == AI_IDLE_HANG)
		Hang();
	_weapons.clear();
	for (unsigned i = 0; i < _config->weapons.size(); i++) {
		Weapon  w = _config->weapons[i];
		w.next_fire = GameTick() + w.cooldown;
		_weapons.push_back(w);
	}
	//Make sure we didn't spawn inside a wall.
	if (!_is_burrowed && !_is_hanging)
		FindOpenSpot();
}

bool Robot::Hit(GLvector2 pos, bool& take_damage)
{
	if (!_bbox.Contains(pos))
		return false;
	//If we have weakspots, then our main body is invulnerable.
	take_damage = true;
	if (_config->has_weakpoints || _config->is_invulnerable)
		take_damage = false;
	for (int i = 0; i < _body_part_count; i++) {
		if (_sprite[i].Collide(pos)) {
			if (take_damage)
				_pain_sprite = i;
			return true;
		}
	}
	if (_config->has_weakpoints) {
		for (unsigned i = 0; i < _wp_sprite.size(); i++) {
			if (_wp_sprite[i].Collide(pos)) {
				take_damage = true;
				_pain_sprite = 0;
				_wp_sprite[i].Shake(GameTick() + 1000);
				_cooldown_stun = GameTick() + 1000;
				return true;
			}
		}
	}
	return false;
}

void Robot::DropPowerups()
{
	int         drop_coins;
	GLvector2   drop_location;

	drop_location = _position;
	//if we die underground, drop stuff at our last valid location.
	if (_is_boreing)
		drop_location = _bore_point;
	drop_coins = _xp;
	//Drop a few extra coins, just to keep the player guessing.
	if (RandomRoll(20))
		drop_coins += RandomVal() % 5;
	EntityXpAdd(drop_location, drop_coins);
	/*
	//If this robot can drop weapons...
	if (!_config->weapon_drops.empty()) {
	int         choose = RandomVal(_config->weapon_drops.size());
	if (choose) {
	fxPickup*   p = new fxPickup;
	p->InitGun(_position, _config->weapon_drops[choose]);
	EntityFxAdd(p);
	}
	}
	*/
	//If this robot uses the drop table...
	if (!_config->drop.empty()) {
		DropLoot(_config->drop, drop_location);
	}
}

GLvector2 Robot::SoundOrigin()
{
	if (_config->is_boss)
		return PlayerPosition();
	return _position;
}

void Robot::Die()
{
	if (_is_dead)
		return;
	if (_config->is_final_boss)
		WorldFinalBossKill();
	_is_dead = true;
	_is_falling = true;
	_death_fade = 1.0f;
	_time_death = GameTick();
	for (int i = 0; i < _wp_sprite.size(); i++)
		_wp_sprite[i].SetColor(GLrgba());
	PlayerScorePoints(_config->score_value);
	PlayerScoreKill();
	DropPowerups();
	//Turn the eye off
	for (int i = 0; i < MAX_PARTS; i++) {
		_sprite[i].EyeEnable(false);
		_sprite[i].SetColor(GLrgba());
	}
	//The legs look awful stretching around during death, so get rid of them
	//and cover the vanishing with particles.
	for (unsigned i = 0; i < _legs.size(); i++) {
		ParticleDebris(_position + _legs[i].Knee(), _config->size, 2);
		ParticleDebris(_position + _legs[i].Knee() / 2.0f, _config->size / 2.0f, 2);
	}
	if (_is_instakilled) {
		ParticleGlow(_position, GLvector2(), _config->body_color, _config->body_color, 6, _config->size);
		ParticleBlood(_position, _config->body_color, _config->size / 1.5f, 5, 2.0f + _config->speed * 100.0f, _at_movement);
		AudioPlay("crash");
		for (int i = 0; i < MAX_PARTS; i++)
			_sprite[i].Init();
	}
	else {
		int particle_count = (int)(_config->size * 10.0f);
		particle_count = clamp(particle_count, 2, 10);
		AudioPlay(_config->sound_die, SoundOrigin());
		ParticleDebris(_position, _config->size / 1.5f, particle_count, 2.0f + _config->speed * 100.0f);
		ParticleDebris(_position, _config->size / 1.5f, particle_count, 100.0f * RandomFloat());
	}
	_legs.clear();
	_death_momentum = _at_movement;
	//if this thing is travelling along the ground, don't let it pop up in the air on death.
	if (_config->body == RBODY_WORM || _config->legs)
		_death_momentum.y = max(_death_momentum.y, 0.0f);
}

void Robot::Damage(int damage)
{
	float         light_fade;

	if (Retired())
		return;
	if (!IsAlerted())
		Alert();
	if (damage < 1 || EnvValueb(ENV_GORG))
		return;

	damage -= _config->armor;
	damage = max(damage, 1);

	_hitpoints -= damage;
	PlayerDealDamage(damage);

	{
		fxMessage* m;
		m = new fxMessage;
		m->Init(_position, "{#f88}-%d", damage);
		EntityFxAdd(m);
	}
	CameraShake(0.15f);
	_cooldown_pain = GameTick() + PAIN_TIME;
	light_fade = (float)_hitpoints / (float)_config->hitpoints;
	for (int i = 0; i < _body_part_count; i++)
		_sprite[i].SetColor(GLrgba(_body_color) * light_fade);
	for (unsigned i = 0; i < _legs.size(); i++)
		_legs[i].ColorSet(GLrgba(_body_color) * light_fade);

	if (_hitpoints <= 0) {
		_pew_pew.clear();
		Die();
	}
	else if (GameTick() > _cooldown_ouch) {
		AudioPlay(_config->sound_hit, SoundOrigin());
		_cooldown_ouch = GameTick() + OUCH_INTERVAL;
	}
}

void Robot::Kick(GLvector2 kick)
{
	float max_distance;

	_impact_kick += kick * KICK_STRENGTH;
	max_distance = _config->size / 2;
	float kick_distance = _impact_kick.Length();
	//Don't get shoved more than a body length off-center.
	if (kick_distance > max_distance) {
		_impact_kick.Normalize();
		_impact_kick *= max_distance;
	}
}

void Robot::Damage(int damage, GLvector2 impact_location, GLvector2 impact_direction)
{
	GLvector2   to_impact;
	float       angle_out;
	float       angle_in;
	float       impact_distance;

	if (Retired() || _is_dead)
		return;
	//If the player one-shots us to death, we will die more violently.
	if (_hitpoints >= _config->hitpoints && damage >= _hitpoints)
		_is_instakilled = true;
	Damage(damage);
	Kick(impact_direction);
	//ParticleSparks(impact_location, impact_direction, _body_color, 7);
	if (!_is_dead)
		return;
	//If the robot is going to vanish, we should leave behind an explosion.
	if (_is_instakilled) {
		fxExplosion*  e;
		e = new fxExplosion;
		e->Init(OWNER_PLAYER, _position, 0);
		e->ColorSet(_body_color, (_body_color + GLrgba(1, 1, 1)) / 2.0f);
		e->SizeSet(_sprite[0].Size() / 2.0f);
		EntityFxAdd(e);
	}
	ParticleDebris(_position, _config->size * DEBRIS_SIZE, 7, 0.05f, impact_direction*0.25f);
	ParticleDebris(_position, _config->size * DEBRIS_SIZE, 2, 0.3f, impact_direction * 2);
	ParticleDebris(_position, _config->size * DEBRIS_SIZE, 1);
	ParticleSmoke(_position, _config->size, 4);
	//ParticleSparks(impact_location, impact_direction * 0.25f, _body_color, 7);
	//ParticleGlow(impact_location, impact_direction * 0.25f, _body_color, _body_color, 3, _config->size * DEBRIS_SIZE);
	_exhaust_point = _position - impact_location;
	to_impact = _position - impact_location;
	//If we were killed by a shot to our extremities, it will look dumb if we
	//emit smoke at this distance.
	impact_distance = to_impact.Length();
	if (impact_distance > _config->size) {
		to_impact /= impact_distance;
		to_impact *= _config->size;
		_exhaust_point = to_impact;
	}
	angle_out = to_impact.Angle();
	angle_in = impact_direction.Angle();
	_death_spin = AngleDifference(angle_out, angle_in) * DEATH_SPIN;
	//Looks dumb to have the jellyfish legs hang down below the spinning body.
	if (_config->body == RBODY_JELLY)
		_death_spin = 0;
}

void Robot::FaceTarget()
{
	_angle = 360 - syMathAngle(_ai_move[MOVE_FORWARD].x, _ai_move[MOVE_FORWARD].y, 0, 0);
}

bool Robot::TryCollide(GLvector2 new_pos, bool avoid_walls)
{
	//Don't even let us get near walls...
	if (avoid_walls)
		return !WorldCellEmpty(new_pos);
	//Regular collision with walls...
	return Collision(new_pos);
}

bool Robot::TryMove(int dir, GLvector2* new_pos)
{
	GLvector2 move;
	float     radius;

	radius = _config->size;
	move = GLvector2(0, 0);
	if (dir == AI_FORWARD)
		move = _ai_move[MOVE_FORWARD];
	if (dir == AI_REVERSE)
		move = _ai_move[MOVE_REVERSE];
	if (dir == AI_SIDE)
		move = _ai_move[MOVE_SIDE];
	if (dir == AI_WANDER)
		move = _ai_move[MOVE_WANDER];
	if (dir == AI_APPROACH)
		move = _ai_move[MOVE_APPROACH];
	if (dir == AI_HOLD)
		move = GLvector2(0, 0);
	if (_config->inertia > 0)
		move = Lerp(move, _inertia, _config->inertia);
	if (_is_pained) {
		float  slow = 1.0f - (float)(_cooldown_pain - GameTick()) / PAIN_TIME;
		move *= 0.3f + slow * 0.7f;
	}
	*new_pos = _position + move;
	if (_config->is_boss)
		return Collision(*new_pos, radius);
	return TryCollide(*new_pos, dir == AI_REVERSE);
}

void Robot::DoMove()
{
	GLvector2   new_pos;
	bool        bump;
	bool				first_choice;
	bool				backing_up;

	bump = true;
	first_choice = true;
	backing_up = false;
	if (_ai_state != AI_IDLE)
		bump = TryMove(_ai_state, &new_pos);
	if (bump) {
		for (int j = 0; j < 4; j++) {
			bump = TryMove(_ai_priorities[j], &new_pos);
			//If we hit a wall trying to move one way, try the other next time.
			if (bump && _ai_priorities[j] == AI_SIDE)
				_ai_flip = !_ai_flip;
			if (!bump) {
				//_ai_cooldown = GameTick () + 1000;
				_ai_state = _ai_priorities[j];
				if (_ai_priorities[j] == AI_REVERSE)
					backing_up = true;
				break;
			}
			first_choice = false;
		}
	}
	if (!bump)
		_position = new_pos;
	if (!first_choice && !backing_up) { //We knocked the wall. Throw off some rubble.
		//ParticleRubble(_position, 0.1f, 4);
		AudioPlay("collide", _position);
	}
}

bool Robot::CanSeePlayer()
{
	GLvector2     offset;
	float         distance;

	if (PlayerIgnore() || !_is_onscreen)
		return false;
	distance = ROBOT_VISION_DISTANCE;
	if (!_is_alerted)
		distance = _config->spot_distance;
	if (_config->is_boss)
		distance *= 2;
	offset = _position - PlayerPosition();
	if (abs(offset.x) > distance || abs(offset.y) > distance)
		return false;
	//If we're burrowed, then look from a spot one body length above us.
	if (_is_burrowed) {
		//We're trying to pop up just as they fly overhead, so ignore the player if they're
		//more than a unit away laterally.
		if (abs(offset.x) > 1)
			return false;
		if (CollisionLos(_position - GLvector2(0, _config->size), PlayerPosition(), LOS_STEP))
			return true;
		return false;
	}
	if (CollisionLos(_position, PlayerPosition(), LOS_STEP))
		return true;
	return false;
}

void Robot::Shove(GLvector2 vector)
{
	_shove += vector;
}

void Robot::Launch(GLvector2 direction)
{
	_launch_inertia = direction.Normalized() * LAUNCH_VELOCITY;
}

bool Robot::DoAttack()
{
	bool      attacked;
	GLvector  camera;
	float     distance;

	//We will shoot at player if we are anywhere on screen.
	distance = CameraPosition().z * 2;
	attacked = false;
	//If we are in range to shoot the player, and we can see them....
	if (_ai_player_distance < distance && CanSeePlayer()) {
		for (unsigned i = 0; i < _weapons.size(); i++) {
			if (GameTick() > _weapons[i].next_fire) {
				const Projectile*   p_info = EnvProjectileFromId(_weapons[i].projectile_id);
				PewPew              pp;
				int                 next_fire;

				next_fire = 0;
				for (int shot = 0; shot < _weapons[i].volley; shot++) {
					next_fire = GameTick() + shot * p_info->_refire_rate;
					//If we're shooting regular projectiles.
					if (_weapons[i].robot_id == -1) {
						pp.p_info = p_info;
						pp.robot_id = -1;
						pp.aim_predicted = _config->attack_predict > 0;
					}
					else { //We're launching robots!
						pp.p_info = NULL;
						pp.robot_id = _weapons[i].robot_id;
						pp.aim_predicted = false;
					}
					pp.body_part = _weapons[i].body_part;
					pp.send_time = next_fire;
					_pew_pew.push_back(pp);
				}
				_weapons[i].next_fire = next_fire + _weapons[i].cooldown + RandomVal(_weapons[i].cooldown);
			}
		}
		attacked = true;
	}
	else { //If we don't have a shot...
		//Halt the weapon cooldown. With will keep this robot from syncing up shots with others of same type.
		for (unsigned i = 0; i < _weapons.size(); i++) {
			_weapons[i].next_fire++;
		}
	}

	if (_config->refire_melee && GameTick() > _cooldown_melee && _ai_player_distance < _config->size * MELEE_REACH && CanSeePlayer()) {
		AudioPlay("saw", SoundOrigin());
		PlayerDamage(_config->melee_damage);
		_cooldown_melee = GameTick() + _config->refire_melee;
		ParticleSparks(_position + _ai_move[MOVE_FORWARD], _body_color, 4);
		if (_config->melee_class == MELEE_EXPLODE) {
			fxExplosion*  e;

			e = new fxExplosion;
			//Explosions attinuate their damage based on distance, but the assumption here is that we're
			//blowing up in the player's face. So we set the explosion to do ZERO damage,
			//and inflict ALL damage below.
			e->Init(OWNER_ROBOTS, _position, 0);
			EntityFxAdd(e);
			_xp = 0;//Don't drop xp if we successfully suicide bomb.
			PlayerDamage(_config->melee_damage);//Inflict direct damage.
			Die();
			Retire();
		}
		//if (_config->melee_class == MELEE_SHOVE)
		PlayerShove(_ai_move[MOVE_FORWARD] * _config->melee_shove_power);
		attacked = true;
	}
	return attacked;
}

void Robot::DoFire()
{
	for (unsigned i = 0; i < _pew_pew.size(); i++) {
		if (_pew_pew[i].send_time <= GameTick()) {
			GLvector2     attack_vector;

			attack_vector = PlayerPosition() - _position;
			if (_pew_pew[i].p_info) { //Launching projectiles.
				if (_pew_pew[i].aim_predicted) {
					PredictPlayer(_pew_pew[i].p_info);
					attack_vector = _player_predicted - _position;
				}
				else  //not a predicted shot
					attack_vector = PlayerPosition() - _position;
				attack_vector.Normalize();

				Kick(attack_vector * -10.0f * _pew_pew[i].p_info->_recoil);

				EntityProjectileFire(OWNER_ROBOTS, _pew_pew[i].p_info, _sprite[_pew_pew[i].body_part].Position(), attack_vector);
			}
			else { //Launching a robot.
				Robot       bot;

				//Only spawn a new bot if we're under the limit.
				if (_children < _config->max_children) {
					bot.Init(_position, _pew_pew[i].robot_id);
					bot.ParentSet(_id);
					bot.Launch(_ai_move[MOVE_SIDE]);
					_children++;
					ParticleSmoke(_position, bot.Size() * 3, 4);
					ParticleGlow(_position, GLvector2(), _body_color, _body_color, 4);
					EntityRobotAdd(bot);
					AudioPlay("missile_big", SoundOrigin());
				}
			}
			_pew_pew.erase(_pew_pew.begin() + i);
			break;
		}
	}
}

void AiTest() {}

void Robot::DoThink()
{
	switch (_config->ai_core) {
	case AI_WALK:
		AiWalk(); break;
	case AI_BEELINE:
		AiBeeline(); break;
	case AI_POUNCE:
		AiPounce(); break;
	case AI_HITNRUN:
		AiHitnrun(); break;
	case AI_ORBIT:
		AiOrbit(); break;
	case AI_SENTRY:
		AiSentry(); break;
	case AI_TUNNEL:
		AiTunnel(); break;
	case AI_GUARD:
		AiGuard(); break;
	case AI_TEST:
		AiTest(); break;
	}
}

void Robot::DoBbox()
{
	_bbox.Clear();
	//Update the bounding box.
	for (int i = 0; i < _body_part_count; i++) {
		GLvector2     size;
		size = GLvector2(_sprite[i].Size(), _sprite[i].Size());
		_bbox.ContainPoint(_sprite[i].Position() - size);
		_bbox.ContainPoint(_sprite[i].Position() + size);
	}
}

void Robot::DoBody()
{
	int           i;

	if (_is_burrowed)
		return;
	if (_is_hanging)
		return;
	_sprite[0].Move(_position + _impact_kick, _angle);
	if (GameTick() < _cooldown_stun || _is_pained) {
		_sprite[0].Blink(0.5f);
	}
	else
		_sprite[0].Blink(0.0f);
	//A floating head with three trailing tentacle "arms".
	if (_config->body == RBODY_SQUID) {
		GLvector2     offset;
		GLvector2     arm[3];
		GLvector2     prev;
		GLvector2     here;
		float         desired_angle;

		//First do the head. Make it turn slowly based on heading.
		desired_angle = _at_movement.Angle();
		_angle += syMathAngleDifference(desired_angle, _angle) * 0.05f;
		_sprite[0].Move(_position + _impact_kick, 180 + _angle);
		//Calculate the attachment points
		offset = GLvectorFromAngle(_angle);
		offset.Normalize();
		offset *= _config->size;
		arm[0] = _position + offset.TurnedRight();
		arm[1] = _position - offset.TurnedRight();
		arm[2] = _position - offset;
		//Create 2 joints to trail behind these anchor points.
		for (i = 0; i < 9; i++) {
			if (!(i % 3)) {
				_anchor[i] = arm[i / 3];
				prev = _anchor[i];
				continue;
			}
			here = _anchor[i];
			offset = here - prev;
			offset.Normalize();
			offset *= _config->size * 2;
			here = prev + offset;
			_anchor[i] = here;
			prev = here;
		}
		for (i = 1; i <= 6; i++) {
			int       arm = (i - 1) / 2;
			int       segment = ((i - 1) % 2);
			GLvector2 anchor, next, mid;

			anchor = _anchor[arm * 3 + segment];
			next = _anchor[arm * 3 + segment + 1];
			mid = (anchor + next) / 2;
			_sprite[i].Move(mid + _impact_kick + _bob, 180 + GLvector2(anchor - next).Angle());
		}
	}
	//Upright head and another sprite that "hangs" below it.
	if (_config->body == RBODY_JELLY) {
		GLvector2     body;

		body = _position + _impact_kick + _bob;
		_sprite[0].Move(body, _angle);
		_sprite[1].Move(body + GLvector2(0, _config->size / 2), (_at_player.x * 25));
	}
	if (_config->body == RBODY_TURRET) {
		GLvector2     body;
		float         angle;
		float         bottom;

		body = _position + _impact_kick + _bob;
		bottom = _config->size / 2;
		angle = _at_player.Angle();
		_sprite[0].Move(body, 0);
		_sprite[1].Move(body + GLvector2(-_config->size, bottom), angle);
		_sprite[2].Move(body + GLvector2(_config->size, bottom), angle);
		_sprite[3].Move(body + GLvector2(0, bottom), 0);
	}
	if (_config->body == RBODY_SNAKE) {
		GLvector2     prev;
		GLvector2     here;
		float         size;
		GLvector2     offset;

		prev = _position;
		for (i = 0; i < _body_part_count; i++) {
			size = _sprite[i].Size();
			if (i == 0)
				size /= 2;
			here = _anchor[i];
			offset = here - prev;
			offset.Normalize();
			offset *= (size / 1.33f);
			here = prev + offset;
			_anchor[i] = here;
			//if we're dead, have the tail go limp and fall on the ground.
			if (_is_dead) {
				_anchor[i] += GRAVITY * 20;
				_anchor[i].y = min(_anchor[i].y, CollisionFloor(_anchor[i]));
			}
			prev = here;
		}
		for (i = 1; i < _body_part_count; i++) {
			here = (_anchor[i - 1] + _anchor[i]) / 2;
			offset = _anchor[i] - _anchor[i - 1];
			_sprite[i].Move(here + _impact_kick, offset.Angle());
		}
	}
	if (_config->body == RBODY_WORM) {
		GLvector2     prev;
		GLvector2     here;
		float         size;
		float         hump;
		GLvector2     offset;

		//when the worm wiggles up and down, this is how much to move.
		hump = _config->size * 0.1f;
		prev = _position;
		for (i = 0; i < _body_part_count; i++) {
			size = _sprite[i].Size();
			if (i == 0)
				size /= 2;
			here = _anchor[i];
			if (i > 0)
				here.y = CollisionFloor(here) - _config->size / 2;
			here.y += SpriteMapVectorFromAngle((int)(here.x * 360)).y * hump;
			offset = here - prev;
			offset.Normalize();
			offset *= (size / 1.5f);
			here = prev + offset;
			_anchor[i] = here;
			prev = here;
		}
		for (i = 1; i < _body_part_count; i++) {
			here = (_anchor[i - 1] + _anchor[i]) / 2;
			offset = _anchor[i] - _anchor[i - 1];
			_sprite[i].Move(here + _impact_kick, offset.Angle());
		}
	}
	for (unsigned i = 0; i < _legs.size(); i++) {
		_legs[i].Move(_position + GLvector2(0, _config->size / 2));
		_legs[i].Update();
	}
	//Move the weak points.
	for (unsigned i = 0; i < _wp_sprite.size(); i++) {
		_wp_sprite[i].Move(_position + _config->weakpoint[i].position);
	}
	DoBbox();
}

void Robot::Alert()
{
	if (_is_hanging) {
		_is_hanging = false;
		ParticleRubble(_position - GLvector2(0, _config->size), _config->size / 3, 3);
	}
	if (_is_burrowed) {
		_is_burrowed = false;
		_position.y -= _config->size;
		ParticleRubble(_position - GLvector2(0, _config->size), _config->size, 6);
		AudioPlay("bore_end", SoundOrigin());
		if (_config->is_boss)
			CameraShake(_config->screen_shake_alert);
	}
	_is_alerted = true;
	_cooldown_charge = 0;
	AudioPlay(_config->sound_see, SoundOrigin());
	for (int i = 0; i < _body_part_count; i++)
		_sprite[i].SetColor(_body_color);
	//The weapon cooldowns don't start until we're alerted.
	for (unsigned i = 0; i < _weapons.size(); i++) {
		_weapons[i].next_fire = GameTick() + _weapons[i].cooldown;
	}
	for (unsigned i = 0; i < _legs.size(); i++) {
		_legs[i].ColorSet(_body_color);
		_legs[i].Extend();
	}
	_sprite[0].EyeEnable(true);
}

void Robot::Update()
{
	GLvector2   old_pos;

	if (Retired())
		return;
	_is_onscreen = RenderPointVisible(_position);
	//if the robot is dead and done crashing, then we can just retire it here.
	if (_is_dead && _parent != 0) {
		Retire();
		return;
	}
	if (_is_dead && _parent == 0 && _children == 0 && !_is_falling && !_is_onscreen && !_config->is_boss) {
		Retire();
		return;
	}
	//If we're a dependant and our parent has died, then we need to kill ourselves.
	if (_config->is_dependant && _parent && GameTick() > _next_dependant_check) {
		Robot*		momma;

		_next_dependant_check = GameTick() + 2000;
		momma = EntityRobotFromId(_parent);
		if (momma == NULL || momma->Dead()) {
			fxExplosion*  e = new fxExplosion;

			e->Init(OWNER_ROBOTS, _position, 0, _sprite[0].Size());
			EntityFxAdd(e);
			Die();
		}
	}

	if (PlayerIgnore())
		_ai_state = AI_WANDER;
	_impact_kick *= KICK_RECOVERY;
	if (_config->ai_core != AI_BEELINE)
		_is_pained = _cooldown_pain > GameTick();
	//Shove is an accumulated vector of which way all the nearby bots want to push us.
	//Turn this into a small movement and apply it before we do any other thinking.
	if (!_shove.IsZero()) {
		GLvector2   nudge;

		_shove.Normalize();
		_shove *= SHOVE_POWER;
		nudge = _position + _shove;
		//Walkers are very particular about where they are on the cell grid, and shoving them into
		//a new cell can trap them or make their legs go crazy. So we won't allow ourselves to be
		//shoved into a new cell.
		if (_config->ai_core == AI_WALK && _is_alerted) {
			if (floor(_position.x) != floor(nudge.x))
				nudge = _position;
			if (floor(_position.y) != floor(nudge.y))
				nudge = _position;
		}
		if (!Collision(nudge, _config->size))
			_position = nudge;
		_shove = GLvector2();
	}
	//Bob up and down slowly.
	if (!_is_dead)
		_bob = Bob(_id * 89 + GameTick());
	//See if we need to give a proximity warning.
	if (_is_alerted && !_is_dead && _config->warn_proximity)
		DoProximity();
	if (_proximity_pulse > 0)
		_proximity_pulse--;
	//If we are occupying a grid tile where WorldCellShape () isn't zero,
	//then we are in a tile that has some sort of wall formation.
	_is_near_wall = WorldCellShape(_position) != 0;
	if (_ai_cooldown == 0) {
		_ai_cooldown = GameTick();
		_cooldown_charge = GameTick() + 1000;
	}
	old_pos = _position;
	//If we're not alerted, then the only thing to do is watch for the player
	if (!_is_alerted) {
		if (CanSeePlayer())
			Alert();
	}
	if (_is_dead)
		AiDeath();
	if (_config->is_boss && _is_alerted && !_is_dead) {
		BossInfo  bi;

		bi.name = _config->name.c_str();
		bi.hp = _hitpoints;
		bi.position = _position - GLvector2(0, _config->size * 1.25f);
		bi.hp_max = _config->hitpoints;
		WorldBossSet(bi);
	}

	if (_is_alerted && !_is_dead && EnvValueb(ENV_AI)) { //Alerted, do AI stuff
		//See if we should be chasing our parent robot...
		if (_config->is_follower && _parent) {
			Robot*		momma = EntityRobotFromId(_parent);
			_ai_move[MOVE_FORWARD] = momma->Position() - _position;
		}
		else //Just chase the player
			_ai_move[MOVE_FORWARD] = PlayerPosition() - _position;
		_ai_move[MOVE_WANDER] = ordinal_direction[_id % 8] * _ai_speed;
		_ai_goal_distance = _ai_move[MOVE_FORWARD].Length();
		_ai_player_distance = GLvector2(PlayerPosition() - _position).Length();
		_ai_move[MOVE_FORWARD].Normalize();
		_at_player = _ai_move[MOVE_FORWARD];
		_ai_move[MOVE_FORWARD] *= _ai_speed;
		_ai_move[MOVE_REVERSE] = _ai_move[MOVE_FORWARD] * -1;
		//Some bots prefer to turn right first...
		if (_ai_flip) {
			_ai_move[MOVE_SIDE].y = _ai_move[MOVE_FORWARD].x;
			_ai_move[MOVE_SIDE].x = -_ai_move[MOVE_FORWARD].y;
		}
		else { //while others are partial to turning left.
			_ai_move[MOVE_SIDE].y = -_ai_move[MOVE_FORWARD].x;
			_ai_move[MOVE_SIDE].x = _ai_move[MOVE_FORWARD].y;
		}
		_ai_move[MOVE_APPROACH] = _ai_move[MOVE_FORWARD] + _ai_move[MOVE_SIDE];
		_ai_move[MOVE_APPROACH].Normalize();
		_ai_move[MOVE_APPROACH] *= _ai_speed;

		for (int i = 0; i < MOVE_COUNT; i++)
			_ai_move[i] += _launch_inertia;
		_launch_inertia *= INERTIA_LOSS;
		DoThink();
		DoFire();
		//Now bump into the player
		if (_ai_player_distance < _config->size  && !PlayerIgnore()) {
			AudioPlay("bump", _position);
			PlayerShove(_ai_move[MOVE_FORWARD] * 0.2f);
		}
	}
	_angle += _death_spin;
	_at_movement = _position - old_pos;
	_inertia = _position - old_pos;
	DoBody();
	MoveEye();
}

//This is called when it's time to make a proximity beep.
void Robot::DoProximity()
{
	float   danger;
	float   pitch;
	int			current_interval;

	//Danger is 1 at point-blank, 0 at distance.
	danger = 1.0f - SmoothStep(_config->size, CameraPosition().z, _ai_player_distance);
	if (danger == 0.0f)
		return;
	//How far apart should the beeps be at this danger level?
	current_interval = 1025 - (int)(1000.0f * (danger));
	if (_last_proximity + current_interval > GameTick())
		return;
	pitch = 1.0f;
	_last_proximity = GameTick();
	if (danger > 0.7f)
		pitch = 1.5f;
	//At max distance, don't actually make the sound.
	if (danger == 1.0f)
		return;
	AudioPlay(_config->proximity_warning, pitch);
	_proximity_pulse = 5;
}

void Robot::MoveEye()
{
	GLvector2     look;

	if (_config->eye_movement == EYEMOVE_SWEEP) {
		look.x = ((float)(GameTick() % 1000) / 500) - 1;
		look.y = 0;
	}
	else if (_config->eye_movement == EYEMOVE_SCAN) {
		look.x = ((float)(GameTick() % 2000) / 500);
		if (look.x > 2)
			look.x = 4 - look.x;
		look.x -= 1;
		look.y = 0;
	}
	else if (_config->eye_movement == EYEMOVE_PLAYER) {
		if (PlayerIgnore())
			look = GLvector2();
		else
			look = _at_player;
	}
	else if (_config->eye_movement == EYEMOVE_HEADING) {
		look = _at_movement;
		look.Normalize();
	}
	_sprite[0].Look(look);
}

void Robot::PredictPlayer(const Projectile*   p)
{
	float               speed;
	float               ticks;
	GLvector2           to_target;
	GLvector2           target;

	speed = p->_speed;
	target = PlayerPosition();
	for (int i = 0; i < 7; i++) {
		//how long will it take our bullet to reach the target?
		to_target = target - _position;
		ticks = to_target.Length() / speed;
		//But where will they be by that time?
		target = PlayerPosition() + PlayerMomentum() * ticks;
	}
	_player_predicted = target;
}

void Robot::RenderHidden()
{
	GLvector2   position;

	if (Dead() || Retired())
		return;
	RenderQuad(_sprite[0].Position(), SPRITE_ALERT, GLrgba(1, 0, 0), _config->size, 0, DEPTH_MESSAGES, true);
	RenderQuad(_sprite[0].Position(), SPRITE_ALERT, GLrgba(1, 0, 0), _config->size, 0, DEPTH_MESSAGES, false);
}

void Robot::RenderBody()
{
	if (Retired())
		return;
	for (unsigned i = 0; i < _legs.size(); i++)
		_legs[i].Render();
	for (unsigned i = 0; i < _wp_sprite.size(); i++)
		_wp_sprite[i].Render();
	if (_config->body != RBODY_TURRET) {//Render in reverse order so the head is drawn on top
		for (int i = (_config->part_count - 1); i >= 0; i--)
			_sprite[i].Render();
	}
	else { //Turrets are drawn so arms go over the body.
		for (int i = 0; i < _config->part_count; i++)
			_sprite[i].Render();
	}
	if (_proximity_pulse) {
		RenderQuad(_sprite[0].Position(), SPRITE_GLOW, GLrgba(1, 0.1f, 0.0f), 1.0f, 2, 0, true);
		RenderQuad(_sprite[0].Position(), SPRITE_GLOW, GLrgba(1, 0.3f, 0.3f), 0.1f, 2, 0, true);
	}
	if (EnvValueb(ENV_BBOX)) {
		glColor3f(1, 1, 0);
		glDisable(GL_TEXTURE_2D);
		_bbox.Render();
		glEnable(GL_TEXTURE_2D);
	}
#ifdef _DEBUG
	RenderDebug();
#endif
	}

void Robot::RenderEye()
{
	if (Retired())
		return;
	_sprite[0].RenderEye();
}

void Robot::RenderIris()
{
	if (Retired())
		return;
	_sprite[0].RenderIris();
}

void Robot::RenderPain()
{
	if (_is_dead || !_is_pained)
		return;
	if (GameFrame() % 2 == 0)
		return;
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glColorMask(false, false, false, false);
	for (int i = (_config->part_count - 1); i >= 0; i--)
		RenderQuad(_sprite[i].Position(), _sprite[i].Sprite(), GLrgba(1, 1, 1), _sprite[_pain_sprite].Size() * 1, _sprite[i].Angle(), DEPTH_PAIN + 0.0f, false);
	RenderQuads();
	glDepthFunc(GL_EQUAL);
	glColorMask(true, true, true, true);
	for (int i = (_config->part_count - 1); i >= 0; i--)
		RenderQuad(_sprite[i].Position(), sprite_square, _body_color * 0.25f, _sprite[i].Size() * 1, _sprite[i].Angle(), DEPTH_PAIN + 0.0f, false);
	RenderQuads();
	glDepthFunc(GL_LEQUAL);
}

void Robot::RenderDebug()
{
#if SHOW_CELL
	GLcoord2    cell = GLcoord2((int)_position.x, (int)_position.y);
	GLvector2   center = GLvector2(cell) + GLvector2(0.5f, 0.5f);

	RenderQuad(center, SPRITE_INVALID, _config->color, 1.0f, 0, 0, true);
#endif

#if SHOW_PREDICTED
	if (_config->ai_core == AI_SNIPER) {
		GLquad    q;
		GLvector2 pt;

		glDisable(GL_TEXTURE_2D);
		glColor3f(1, 1, 0);
		glBegin(GL_QUADS);
		q = SpriteQuad(0);
		pt = _player_predicted;
		for (int j = 0; j < 4; j++) {
			glVertex3f(pt.x + q.corner[j].x * 0.1f, pt.y + q.corner[j].y * 0.1f, DEPTH_UNITS);
		}
		glEnd();
		glEnable(GL_TEXTURE_2D);
}
#endif

#if SHOW_LAUNCHERS
	//Render attachment points between sprites. Used in debugging.
	for (int i = 8; i >= 0; i--)  {
		RenderQuad(_anchor[i], SPRITE_TARGET, GLrgba(1, 1, 1), _config->size / 4, 0, DEPTH_PROJECTILES, false);
	}
	for (unsigned i = 0; i < _config->launch_point.size(); i++) {
		GLquad    q;
		GLvector2 pt;

		glDisable(GL_TEXTURE_2D);
		glColor3f(1, 0, 0);
		pt = _config->launch_point[i];
		RenderQuad(_position + pt, SPRITE_TARGET, GLrgba(1, 1, 1), 0.2f, 0, DEPTH_PROJECTILES, true);
	}
	for (unsigned i = 0; i < _config->laser_point.size(); i++) {
		GLvector2 pt;

		pt = _config->laser_point[i];
		RenderQuad(_position + pt, SPRITE_TARGET, GLrgba(1, 0, 0), 0.2f, 0, DEPTH_PROJECTILES, true);
	}
	for (unsigned i = 0; i < _config->laser_point.size(); i++) {
		GLquad    q;
		GLvector2 pt;

		glDisable(GL_TEXTURE_2D);
		glColor3f(0, 0, 1);
		glBegin(GL_QUADS);
		q = SpriteQuad(0);
		pt = _config->laser_point[i];
		RenderQuad(_position + pt, SPRITE_TARGET, GLrgba(1, 1, 1), 0.2f, 0, DEPTH_PROJECTILES, true);
		for (int j = 0; j < 4; j++) {
			glVertex3f(_position.x + pt.x + q.corner[j].x * 0.1f, _position.y + pt.y + q.corner[j].y * 0.1f, DEPTH_UNITS);
		}
		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
#endif
}