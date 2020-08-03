#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "audio.h"

enum ProjectileType
{
	PROJECTILE_BOLT,
	PROJECTILE_BEAM,
};

enum ProjectileSlot
{
	PROJECTILE_ROBOT,
	PROJECTILE_PRIMARY,
	PROJECTILE_SECONDARY,
	PROJECTILE_SLOTS,
};

enum HandAnimate
{
	HAND_FIXED,
	HAND_AIM,
	HAND_SPIN
};

class Projectile
{
public:
	string            _name;
	string            _title;
	string            _description;
	ProjectileSlot    _slot;
	enum SoundLoop    _sound_loop;
	int               _id;
	ProjectileType    _type;
	HandAnimate       _hand_animate;
	GLvector2         _size;
	GLrgba            _color;
	GLrgba            _aura_color;
	SpriteEntry       _sprite;
	SpriteEntry       _icon;
	SpriteEntry       _hand;
	bool              _is_glowing;
	bool              _is_homing;
	bool              _is_weightless;
	bool              _is_shootable;
	bool              _has_aura;
	bool              _has_acceleration;
	bool              _friendly_fire;
	bool              _is_oneshot;
	bool              _is_volatile;
	float             _aura_size;
	float             _explosion_radius;
	float             _gravity;
	float             _recoil;
	float             _scatter;
	float             _speed;
	float             _start_speed;
	float             _spin;
	float             _tail_brightness;
	float             _turn_rate;
	int               _bounces;
	int								_penetration;
	int               _damage;
	int               _debris;
	int               _refire_rate;
	int               _spark_interval;
	int               _tick_cooldown;
	long              _tick_lifespan;
	long              _tick_fadeout;
	long              _tick_warmup;
	long              _tick_accelerate;
	string            _sound;
	int               _volley;
	int               _cost;
	float             _screen_shake;

	void              Init(class iniFile&, string name);

private:
	ProjectileType    TypeFromString(string s);
	ProjectileSlot    SlotFromString(string s);
	HandAnimate       HandAnimateFromString(string s);
};

#endif // PROJECTILE_H
