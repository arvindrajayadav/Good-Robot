/*-----------------------------------------------------------------------------

  Robot_ai.cpp

  These are the AI routines for the robot class.  The REST of the class lives
  in robots.cpp, but the AI is put here to make the code more managable.

  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#define RANGES            (sizeof (shoot_range) / sizeof (float))
#define AT_REST           0.01f

#include "audio.h"
#include "bodyparts.h"
#include "camera.h"
#include "collision.h"
#include "entity.h"
#include "env.h"
#include "fx.h"
#include "game.h"
#include "page.h"
#include "particle.h"
#include "player.h"
#include "random.h"
#include "robot.h"
#include "sprite.h"
#include "world.h"

static float              shoot_range[] = { -0.5f, 0.0f, 0.5f, 1.25f };

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static bool is_floor(short shape)
{
	if (shape == 8 || shape == 4 || shape == 12 || shape == 13 || shape == 14)
		return true;
	return false;
}

//This AI will take a potshot at the player, then run away until ready to
//fire again. Works best if the foe has a powerful attack with a long cooldown.
void Robot::AiHitnrun()
{
	bool    ready_to_attack;

	_ai_speed = _config->speed;
	if (GameTick() > _ai_cooldown || _ai_state == AI_IDLE) {
		ready_to_attack = WantToAttack();
		if (_config->refire_melee && GameTick() > _cooldown_melee)
			ready_to_attack = true;
		//If we're ready to shoot, head for the player.
		if (ready_to_attack) {
			_ai_speed = _config->speed;
			_ai_priorities[0] = AI_FORWARD;
			_ai_priorities[1] = AI_SIDE;
			_ai_priorities[2] = AI_REVERSE;
			_ai_priorities[3] = AI_WANDER;
		}
		else { //We're running away.
			_ai_speed = _config->speed;
			_ai_priorities[0] = AI_REVERSE;
			_ai_priorities[1] = AI_SIDE;
			_ai_priorities[2] = AI_WANDER;
		}
		_ai_cooldown = GameTick() + 250;
		_ai_state = AI_IDLE;
	}
	//Don't need this per-frame. Used only in testing.
	//PredictPlayer (now);
	DoMove();
	DoAttack();
}

//This AI will orbit around the player, shooting at them. Works best with
//AI that, you know, SHOOT.
void Robot::AiOrbit()
{
	float     range;
	int       range_select;

	_ai_speed = _config->speed;
	range_select = _id % RANGES;
	range = _config->attack_range + shoot_range[range_select];
	//Time to think about what we want to be doing.
	if (GameTick() > _ai_cooldown || _ai_state == AI_IDLE) {
		if (_ai_goal_distance < range - SHOOT_ZONE) {
			_ai_priorities[0] = AI_REVERSE;
			_ai_priorities[1] = AI_SIDE;
			_ai_priorities[2] = AI_FORWARD;
			_ai_priorities[3] = AI_WANDER;
		}
		else if (_ai_goal_distance > range + SHOOT_ZONE) {
			_ai_priorities[0] = AI_FORWARD;
			_ai_priorities[1] = AI_SIDE;
			_ai_priorities[2] = AI_REVERSE;
			_ai_priorities[3] = AI_WANDER;
		}
		else {
			_ai_priorities[0] = AI_SIDE;
			_ai_priorities[1] = AI_REVERSE;
			_ai_priorities[2] = AI_FORWARD;
			_ai_priorities[3] = AI_WANDER;
		}
		_ai_cooldown = GameTick() + 200;
		_ai_state = AI_IDLE;
	}
	DoMove();
	DoAttack();
}

void Robot::AiGuard()
{
	GLvector2 to_target;
	GLvector2 player;
	GLvector2 page_center;
	GLvector2 target;
	GLvector2 move;
	float			distance;

	player = PlayerPosition();
	to_target = GLvector2(player - _position);
	distance = to_target.Length();
	//Figure out which page we're on.
	page_center = _position / PAGE_SIZE;
	page_center.x = floor(page_center.x);
	page_center.y = floor(page_center.y);
	page_center = page_center * PAGE_SIZE + GLvector2(PAGE_HALF, PAGE_HALF);
	to_target.Normalize();
	distance = min(distance, (float)PAGE_HALF);
	target = page_center + to_target * (distance / 2);
	move = target - _position;
	_ai_speed = _config->speed;
	move = move.Normalized() * _ai_speed;
	move = Lerp(move, _inertia, 0.99f);

	_position += move;
	if (DoAttack())
		_ai_flip = !_ai_flip;
}

//This AI will head for a point directly below the player, or
//directly above. When it passes the player (moves from their
//left to their right) it reverses these.
void Robot::AiTunnel()
{
	float     to_target;
	float     turn;
	float     max_turn;
	GLvector2 target;
	GLvector2 player;
	GLvector2 movement;
	bool      go_high;
	bool      boreing;

	boreing = Collision(_position);
	player = PlayerPosition();
	//We're either entering or exiting
	if (_is_boreing != boreing) {
		ParticleRubble(_position, _config->size, 6);
		AudioPlay("bore_end", SoundOrigin());
		if (_config->is_boss)
			CameraShake(_config->screen_shake_bore);
		_is_boreing = boreing;
	}
	max_turn = 3;
	if (boreing)
		max_turn = 1;
	go_high = false;
	if (player.x < _position.x)
		go_high = true;
	if (_ai_flip)
		go_high = !go_high;
	if (go_high)
		target = player + GLvector2(0, _config->size * 10);
	else
		target = player + GLvector2(0, -_config->size * 10);
	to_target = GLvector2(target - _position).Angle();
	turn = syMathAngleDifference(_angle, to_target);
	turn = clamp(turn, -max_turn, max_turn);
	_angle += turn;
	movement = GLvectorFromAngle(_angle + 180) * _ai_speed;
	_position += movement;
	//If we're not tunneling, then we're free to shoot.
	if (!boreing) {
		_ai_speed = _config->speed;
		_bore_point = _position;
		if (DoAttack())
			_ai_flip = !_ai_flip;
	}
	else { //we're tunneling
		//All weapons are on half-cooldown when we emerge, so we don't always insta-fire on exit.
		for (unsigned i = 0; i < _weapons.size(); i++)
			_weapons[i].next_fire = GameTick() + _weapons[i].cooldown / 2;
		//Except for melee, which is instantly ready when we start digging.
		_cooldown_melee = 0;
		ParticleRubble(_bore_point, _config->size / 3, 1);
		_ai_speed = _config->speed / 2;
		if (GameTick() > _cooldown_ouch) {
			_cooldown_ouch = GameTick() + 1000;
			AudioPlay("bore", _position);
		}
	}
}

void Robot::AiSentry()
{
	_ai_speed = _config->speed;
	if (GameTick() > _ai_cooldown || _ai_state == AI_IDLE) {
		if (_ai_goal_distance < 1.5f) { //Too close. Back away.
			_ai_priorities[0] = AI_REVERSE;
			_ai_priorities[1] = AI_SIDE;
			_ai_priorities[2] = AI_HOLD;
			_ai_priorities[3] = AI_FORWARD;
		}
		else if (_ai_goal_distance > 5) { //Too far. Close in.
			_ai_priorities[0] = AI_FORWARD;
			_ai_priorities[1] = AI_SIDE;
			_ai_priorities[2] = AI_REVERSE;
			_ai_priorities[3] = AI_HOLD;
		}
		else if (CollisionLos(_position, PlayerPosition(), LOS_STEP)) { //Hold so we can shoot.
			_ai_priorities[0] = AI_HOLD;
			_ai_priorities[1] = AI_REVERSE;
			_ai_priorities[2] = AI_FORWARD;
			_ai_priorities[3] = AI_WANDER;
		}
		else { //We're in rage but can't see the player. Move around for better position.
			_ai_priorities[0] = AI_SIDE;
			_ai_priorities[1] = AI_REVERSE;
			_ai_priorities[2] = AI_FORWARD;
			_ai_priorities[3] = AI_HOLD;
		}
		_ai_cooldown = GameTick() + 50;
		_ai_state = AI_IDLE;
	}
	DoMove();
	DoAttack();
}

//This is used by dying robots as they crash into the ground.
void Robot::AiDeath()
{
	GLvector2   smoke_out;
	GLvector2   norm;
	GLvector2   old_position;
	float       depth;

	old_position = _position;
	if (_is_falling) {
		_position += _death_momentum;
		_death_momentum.y += GRAVITY;
		if (GameTick() > _cooldown_smoke) {
			_cooldown_smoke = GameTick() + 32;
			smoke_out = _position + SpriteMapVectorRotate(_exhaust_point, (int)_angle);
			ParticleSmoke(smoke_out, _config->size * 2, 2);
		}
	}
	else { //We've hit the ground and come to rest.
		if (GameTick() > _cooldown_smoke) {
			_cooldown_smoke = GameTick() + 1000;
			smoke_out = _sprite[RandomVal(_body_part_count)].Position();
			ParticleSmoke(smoke_out, _config->size * 2, 2);
		}
	}
	_death_fade *= 0.97f;
	if (Collision(_position, &norm, &depth)) {
		_position = old_position;
		norm.Normalize();
		_death_momentum = GLreflect2(_death_momentum, norm) * 0.3f;
		if (_death_momentum.Length() > AT_REST) {
			_death_spin = _death_momentum.x * 45.0f;
			AudioPlay("crash", SoundOrigin());
			ParticleDebris(_position, _config->size * 2, 7);
		}
		else {
			_is_falling = false;
			_death_spin = 0.0f;
			_body_color = GLrgba();
			for (int i = 0; i < _body_part_count; i++)
				_sprite[i].SetColor(GLrgba());
		}
	}
	if (IsBoss() && _is_falling) {
		for (unsigned i = 0; i < MAX_PARTS; i++) {
			_sprite[i].SetColor(GLrgba());
			_sprite[i].EyeEnable(false);
			_sprite[i].Move(_position);
		}
		if (GameTick() > _ai_cooldown) {
			fxExplosion*  e;

			e = new fxExplosion;
			e->Init(OWNER_ROBOTS, _position, _config->melee_damage);
			EntityFxAdd(e);
			ParticleDebris(_position, _config->size / 3, 10);
			AudioPlay("explosion", _position);
			_ai_cooldown = GameTick() + 500;
		}
	}
}

void Robot::AiPounce()
{
	bool      touching;
	GLvector2 reach;

	if (GameTick() > _ai_cooldown || _ai_state == AI_IDLE) {
		_ai_speed = _config->speed;
		if (_ai_goal_distance <= 1.5f) {
			_ai_speed = _config->speed * 2;
			_ai_priorities[0] = AI_FORWARD;
			_ai_priorities[1] = AI_SIDE;
			_ai_priorities[2] = AI_REVERSE;
			_ai_priorities[3] = AI_WANDER;
		}
		else if (_ai_goal_distance > 3) {
			_ai_priorities[0] = AI_SIDE;
			_ai_priorities[1] = AI_REVERSE;
			_ai_priorities[2] = AI_FORWARD;
			_ai_priorities[3] = AI_WANDER;
		}
		else {
			_ai_priorities[0] = AI_APPROACH;
			_ai_priorities[1] = AI_FORWARD;
			_ai_priorities[2] = AI_SIDE;
			_ai_priorities[3] = AI_REVERSE;
		}
		_ai_cooldown = GameTick() + 1000;
		_ai_state = AI_IDLE;
	}
	DoMove();
	FaceTarget();
	touching = false;
	DoAttack();
}

//Head directly for the player. Best used with MELEE attacks.
void Robot::AiBeeline()
{
	GLvector2 reach;

	_ai_speed = _config->speed;
	_ai_priorities[0] = AI_FORWARD;
	_ai_priorities[1] = AI_SIDE;
	_ai_priorities[3] = AI_WANDER;
	_ai_priorities[2] = AI_REVERSE;
	_ai_state = AI_IDLE;
	_ai_cooldown = 0;
	DoMove();
	DoAttack();
	_angle = fmod(_angle + 10.0f, 360.0f);
}

void Robot::AiBossReflector()
{
	GLvector2     player;

	player = PlayerPosition();
	//It's frustrating fighting this guy when he clings to the bottom of the screen, so
	//if we're below the player, then rush them so they can get under us again.
	if (_position.y > player.y + 1.0f) {
		_ai_priorities[0] = AI_FORWARD;
		_ai_priorities[1] = AI_SIDE;
		_ai_priorities[2] = AI_HOLD;
	}
	else { //player is below us
		if (_ai_goal_distance < 3.2f) { //Too close. Back away.
			_ai_priorities[0] = AI_REVERSE;
			_ai_priorities[1] = AI_SIDE;
			_ai_priorities[2] = AI_HOLD;
		}
		else if (_ai_goal_distance > 5.5f) { //Too far. Close in.
			_ai_priorities[0] = AI_FORWARD;
			_ai_priorities[1] = AI_SIDE;
			_ai_priorities[2] = AI_HOLD;
		}
		else if (CollisionLos(_position, PlayerPosition(), LOS_STEP)) { //Hold so we can shoot.
			_ai_priorities[0] = AI_SIDE;
			_ai_priorities[1] = AI_REVERSE;
			_ai_priorities[2] = AI_HOLD;
		}
		else { //We're in range but can't see the player. Wait for them to show up.
			_ai_priorities[0] = AI_HOLD;
		}
	}
	_ai_cooldown = GameTick() + 250;
	_ai_state = AI_IDLE;
	DoMove();
	DoAttack();
}

bool Robot::WantToAttack()
{
	if (_config->refire_melee && GameTick() > _cooldown_melee)
		return true;
	for (unsigned i = 0; i < _weapons.size(); i++) {
		if (GameTick() > _weapons[i].next_fire) {
			return true;
		}
	}
	return false;
}

//This is used by AI that travels along the ground, moving one or more
//single-joined insect legs.
void Robot::AiWalk()
{
	float             floor;
	float             new_floor;
	float             desired_height;
	GLvector2         move;
	GLvector2         new_pos;
	bool              los;
	bool              want_to_move;
	bool              want_to_attack;
	bool              want_to_stand;
	float             standing;
	float             crouching;

	los = CanSeePlayer();
	move = GLvector2();
	want_to_attack = WantToAttack();
	want_to_move = false;
	want_to_stand = false;
	//If we want to attack but we can't see the player, or we DON'T want to attack
	//but we DO see the player, then we need to move.
	if (want_to_attack != los)
		want_to_move = true;
	floor = CollisionFloor(_position);
	standing = floor - _config->walk_stand * _config->size;
	crouching = floor - _config->walk_crouch * _config->size;
	//If we're not on flat ground, then we need to be standing to get over the ramp or whatever.
	short shape = WorldCellShape(GLvector2(_position.x, floor));
	if (shape != 12)
		want_to_stand = true;
	if (want_to_attack || !_pew_pew.empty())
		want_to_stand = true;
	//If we've got shots in the queue,  then we want to stand.
	if (!_pew_pew.empty())
		want_to_stand = true;
	_ai_speed = _config->speed;
	if (!want_to_attack)
		_ai_speed *= 2;
	//Try to get out of sight
	if (want_to_move) {
		if (_ai_flip) { //Move towards player.
			move.x = PlayerPosition().x - _position.x;
			if (abs(move.x) < 0.5f)
				move.x = 0;
		}
		else
			move.x = _position.x - PlayerPosition().x;
		move.x = SIGNF(move.x);
	}
	if (want_to_stand) {
		_ai_speed = _config->speed;
		desired_height = standing;
	}
	else {
		_ai_speed = _config->speed * 2;
		desired_height = crouching;
	}
	move.y = SIGNF(desired_height - _position.y) * 3;
	move.Normalize();
	move *= _ai_speed;
	new_pos = _position + move;
	new_pos.y = clamp(new_pos.y, standing, crouching);
	new_floor = CollisionFloor(new_pos);
	if (abs(new_floor - floor) >= 0.4f) {
		new_pos = _position;
		_ai_flip = !_ai_flip;
	}
	if (Collision(new_pos, _config->size / 2)) {
		_ai_flip = !_ai_flip;
		new_pos = _position;
	}
	//Do special checks to make sure the legs move reasonably.
	if (_legs.size() >= 2) {
		GLvector2 offsets[3] = { GLvector2(0, 0), GLvector2(0, 1), GLvector2(0, 2) };
		bool      move_right, move_left;
		int       right, left;
		short     shape;

		//Legs are originally constructed starting with the innermost. This means
		//that the last two legs are the ones furthest from the body. We use those
		//for collision checking. Right legs come before left.
		right = _legs.size() - 2;
		left = _legs.size() - 1;
		//Odd numbered legs? Then the last leg is the middle one. Skip it.
		if (_legs.size() % 2) {
			right--;
			left--;
		}
		move_right = false;
		for (unsigned step = 0; step < 3; step++) {
			shape = WorldCellShape(new_pos + _legs[right].Bumper() + offsets[step]);
			if (is_floor(shape)) {
				move_right = true;
				break;
			}
		}
		if (!move_right && new_pos.x > _position.x)
			new_pos.x = _position.x;
		move_left = false;
		for (unsigned step = 0; step < 3; step++) {
			shape = WorldCellShape(new_pos + _legs[left].Bumper() + offsets[step]);
			if (is_floor(shape)) {
				move_left = true;
				break;
			}
		}
		if (!move_left && new_pos.x < _position.x)
			new_pos.x = _position.x;
		if (!move_left || !move_right)
			_ai_flip = !_ai_flip;
	}
	//We passed all those checks, so it's okay to move.
	_position = new_pos;
	if (want_to_attack) {
		DoAttack();
		//Commented out code below as assigning to self does nothing.
		//if (DoAttack ())
		//  _ai_flip = _ai_flip;
	}
}