/*-----------------------------------------------------------------------------

PlayerWeapon.cpp

This manages the player weapons - Their refire, their cooldowns, and their
firing logic. This directly communicates with the playerstats, so it's not
appropriate for use with robots.

Good Robot
(c) 2015 Pyrodactyl

-----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "entity.h"
#include "game.h"
#include "particle.h"
#include "player.h"
#include "playerweapon.h"
#include "projectile.h"

//Internal scaling. Use gameplay.ini to modify the in-game recoil value.
#define BULLET_RECOIL       -0.1f
#define MIN_WEAPON_GLOW     0.33f
#define MAX_WEAPON_SPIN     10.0f

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void PlayerWeapon::Fire(GLvector2 origin, GLvector2 shoot)
{
	int               now;
	int               refire;

	now = GameTick();
	//Don't use refire for one-shot weapons. Otherwise, it's possible to have the weapon
	//spin all the way up and then fail to fire. This will feel like a bug.
	if (!_p_info->_is_oneshot) {
		refire = _p_info->_refire_rate * EnvPlayerRefire(Player()->Skill(SKILL_SHOT_SPEED)).val;
		_shot_refire = now + refire;
	}
	EntityProjectileFire(OWNER_PLAYER, _p_info, origin, shoot, Player()->Skill(SKILL_SHOT_POWER));
	PlayerShove(shoot * _p_info->_recoil * BULLET_RECOIL);
}

void PlayerWeapon::Equip(const Projectile* p_info)
{
	_firing = false;
	_p_info = p_info;
	_shot_refire = 0;
	_shot_warmup = 0;
	_weapon_warmup = 0.0f;
	_weapon_cooldown = 0.0f;
	_cooldown_expires = 0;
	_weapon_spin = 0.0f;
	_weapon_angle = 0.0f;
	_weapon_glow = GLrgba();
}

//Zero is ready, 1.0 is cooling off.
float PlayerWeapon::Cooldown()
{
	if (!Ready()) //No weapon equipped.
		return 0.0f;
	if (_p_info->_tick_cooldown == 0) //If no cooldown, then always ready.
		return 0.0f;
	return _weapon_cooldown;
}

void PlayerWeapon::Update(GLvector2 origin, bool trigger_down)
{
	GLvector2         shoot;
	int               now;
	int               power;

	//We can't do anything if we don't have a weapon equipped.
	if (!Ready())
		return;
	now = GameTick();
	_fired_this_frame = false;
	//Safety cheat code disables weapon fire, for demo / testing / trailers.
	if (EnvValueb (ENV_SAFETY))
		trigger_down = false;
	//If this weapon has a cooldown...
	if (_p_info->_tick_cooldown > 0) {
		_weapon_cooldown = (float)(_cooldown_expires - now) / (float)_p_info->_tick_cooldown;
		_weapon_cooldown = clamp(_weapon_cooldown, 0.0f, 1.0f);
	}
	_weapon_angle += _weapon_spin;
	if (_weapon_angle > 360.0f)
		_weapon_angle -= 360.0f;
	_weapon_spin *= 0.95f;
	if (_weapon_spin > MAX_WEAPON_SPIN)
		_weapon_spin *= 1.0f;
	_weapon_glow = GLrgba(_p_info->_color) * MIN_WEAPON_GLOW;
	//Player isn't holding down the fire button.
	if (!trigger_down) {
		//If we WERE firing, puff out some particles now that we've stopped.
		if (_firing)
			ParticleSparks(origin, _weapon_glow, 4);
		_firing = false;
		_shot_warmup = now + _p_info->_tick_warmup;
		AudioLoop(_p_info->_sound_loop, "overcharge", 0.5f, 0.0f);
		return;
	}
	//If this is a "one shot" weapon then we need to take our finger off the trigger before we can fire again...
	if (_firing && _p_info->_is_oneshot)
		return;
	//If this weapon has a cooldown that hasn't expired, then we can't fire yet.
	if (now < _cooldown_expires) {
		AudioPlay("missile_out");
		_firing = true;
		return;
	}
	//We're holding down the fire button, but the weapon isn't done spinning up yet.
	if (now < _shot_warmup && _p_info->_tick_warmup > 0) {
		_weapon_warmup = 1.0f - (float)(_shot_warmup - now) / (float)_p_info->_tick_warmup;
		_weapon_glow = GLrgba(_p_info->_color) * (MIN_WEAPON_GLOW + _weapon_warmup);
		_weapon_spin = MAX_WEAPON_SPIN * _weapon_warmup;
		AudioLoop(_p_info->_sound_loop, "overcharge", 0.5f + _weapon_warmup, 1.0f);
		return;
	}
	if (!_firing)
		_fired_this_frame = true;
	_firing = true;
	_weapon_spin = MAX_WEAPON_SPIN;
	_weapon_warmup = 1.0f;
	_weapon_glow = GLrgba(_p_info->_color);
	AudioLoop(_p_info->_sound_loop, "overcharge", 0.5f, 0.0f);
	//Fire button is down, weapon is hot, but we're waiting for the refire timer.
	if (now < _shot_refire)
		return;
	//Finally ready to shoot some projectiles.
	power = Player()->Skill(SKILL_SHOT_POWER);
	Player()->TriviaSet(TRIVIA_BULLETS_FIRED, Player()->Trivia(TRIVIA_BULLETS_FIRED) + 1);
	shoot = PlayerAim() - origin;
	shoot.Normalize();
	Fire(origin, shoot);
	ParticleBloom(origin, Color(), 0.5f, 150);
	//If this weapon has a cooldown...
	if (_p_info->_tick_cooldown > 0) {
		_firing = false;
		_cooldown_expires = now + _p_info->_tick_cooldown;
	}
}