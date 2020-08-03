/*-----------------------------------------------------------------------------

Projectile.cpp

A class to load and track the properties of the various projectiles
used by both the player and the other robots.

Good Robot
(c) 2015 Pyrodactyl

-----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "ini.h"
#include "projectile.h"

static int      id;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

ProjectileType Projectile::TypeFromString(string s)
{
	if (!StringComparei(s, "bolt"))
		return PROJECTILE_BOLT;
	if (!StringComparei(s, "beam"))
		return PROJECTILE_BEAM;
	return PROJECTILE_BOLT;
}

ProjectileSlot Projectile::SlotFromString(string name)
{
	if (!StringComparei(name, "primary"))
		return PROJECTILE_PRIMARY;
	if (!StringComparei(name, "secondary"))
		return PROJECTILE_SECONDARY;
	return PROJECTILE_ROBOT;
}

HandAnimate Projectile::HandAnimateFromString(string s)
{
	if (!StringComparei(s, "spin"))
		return HAND_SPIN;
	if (!StringComparei(s, "aim"))
		return HAND_AIM;
	return HAND_FIXED;
}

void Projectile::Init(class iniFile& f, string name)
{
	_name = name;
	_id = id;
	id++;
	_title = f.StringGet(_name, "Title");
	_description = f.StringGet(_name, "Description");
	if (_title.empty())
		_title = _name;
	_type = TypeFromString(f.StringGet(_name, "Type"));
	_slot = SlotFromString(f.StringGet(_name, "Slot"));
	_sound_loop = _slot == PROJECTILE_PRIMARY ? LOOP_PRIMARY : LOOP_SECONDARY;
	_size = f.Vector2Get(_name, "Size");
	_color = GLrgbaFromHex(f.StringGet(_name, "Color"));
	_aura_color = GLrgbaFromHex(f.StringGet(_name, "AuraColor"));
	_sprite = SpriteEntryLookup(f.StringGet(_name, "Sprite"));
	_icon = SpriteEntryLookup(f.StringGet(_name, "Icon"));
	_hand = SpriteEntryLookup(f.StringGet(_name, "Hand"));
	_hand_animate = HandAnimateFromString(f.StringGet(_name, "HandAnimate"));

	_aura_size = f.FloatGet(_name, "AuraSize");
	_explosion_radius = f.FloatGet(_name, "ExplosionRadius");
	_gravity = f.FloatGet(_name, "Gravity");
	_recoil = f.FloatGet(_name, "Recoil");
	_scatter = f.FloatGet(_name, "Scatter");
	_speed = f.FloatGet(_name, "Speed");
	_start_speed = f.FloatGet(_name, "StartSpeed");
	_spin = f.FloatGet(_name, "Spin");
	_tail_brightness = f.FloatGet(_name, "Tail");
	_turn_rate = f.FloatGet(_name, "TurnRate");
	_screen_shake = f.FloatGet(_name, "ScreenShake");

	_sound = f.StringGet(_name, "Sound");
	_bounces = f.IntGet(_name, "Bounces");
	_penetration = max(1, f.IntGet(_name, "Penetration"));
	_damage = f.IntGet(_name, "Damage");
	_debris = f.IntGet(_name, "Debris");
	_refire_rate = max(1, f.IntGet(name, "RefireRate"));
	_spark_interval = f.IntGet(_name, "SparkInterval");
	_tick_lifespan = max(1l, f.LongGet(name, "Lifespan"));
	_tick_fadeout = f.LongGet(_name, "fadeout");
	_tick_warmup = f.LongGet(_name, "Warmup");
	_tick_cooldown = f.LongGet(_name, "Cooldown");
	_tick_accelerate = f.LongGet(_name, "AccelerationTime");

	_volley = f.IntGet(_name, "Volley");

	_is_homing = f.BoolGet(_name, "Homing");
	_is_volatile = f.BoolGet(_name, "Volatile");
	_is_oneshot = f.BoolGet(_name, "Oneshot") || (_tick_cooldown > 0);
	_is_shootable = f.BoolGet(_name, "Shootable");
	_is_glowing = f.BoolGet(_name, "Glow");
	_has_acceleration = _tick_accelerate > 0;
	_friendly_fire = f.BoolGet(_name, "FriendlyFire");
	_cost = f.IntGet(_name, "Cost");
	_is_weightless = _gravity == 0.0f;
	_has_aura = _aura_size > 0.0f;
}