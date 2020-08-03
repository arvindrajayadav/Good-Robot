#ifndef FX_H
#define FX_H

#include "game.h"
#include "bodyparts.h"
#include "interface.h"
#include "projectile.h"

enum fxType
{
	FX_NONE,
	FX_DOOR,
	FX_POWERUP,
	FX_EXPLOSION,
	FX_SHIELDHIT,
	FX_ACCESS,
	FX_MESSAGE,
	FX_TITLE,
	FX_PROJECTILE,
	FX_MACHINE,
	FX_FORCEFIELD,
	FX_HAT,
};

class fx
{
protected:
	GLvector2           _origin;
	int                 _frame;
	int                 _id;
	bool                _active;
public:
	virtual             ~fx()     {}
	bool                Active() { return _active; }
	void                Retire() { _active = false; }
	virtual void        Update() = 0;
	virtual void        Render() = 0;
	virtual fxType      Type() = 0;
};

enum fxOwner
{
	OWNER_NONE,
	OWNER_PLAYER,
	OWNER_ROBOTS,
};

enum fxMissileType
{
	MISSILE_DUMBFIRE,
	MISSILE_HOMING,
	MISSILE_CLUSTER,
	MISSILE_NUKE,
	MISSILE_SMARTBOMB,
};

enum PowerupType
{
	POWERUP_COIN,
	POWERUP_SHIELD,
	POWERUP_MISSILES
};

class fxPowerup : public fx
{
	PowerupType       _type;
	GLvector2         _momentum;
	GLrgba            _color;
	GLrgba            _color_particle1;
	GLrgba            _color_particle2;
	enum SpriteEntry  _sprite;
	bool              _gathering;
	float             _size;
	float             _phase;
	int               _animate_offset;
	int               _angle;
	int               _angle2;
	int               _begin;
	int               _value;
	int               _lifespan;
	int               _force_gather;
	int               _particle_cooldown;

public:
	void              Init(GLvector2 position, PowerupType type, int value = 0);
	void              Render();
	GLvector2         Position() { return _origin; }
	int               Value() { return _value; }
	void              Update();
	void              Gather() { _gathering = true; }
	fxType            Type() { return FX_POWERUP; }
};

class fxShieldHit : public fx
{
	GLvector2         _origin;
	GLrgba            _start_color;
	GLrgba            _color;
	bool              _use_hit;
	int               _angle;
	float             _size;
	float             _bright;
	int               _begin;
	GLquad            _quad;

public:
	void              Init(GLrgba color, int timestamp, float size);
	void              HitAngle(int angle);
	void              Update();
	void              Render();
	fxType            Type() { return FX_SHIELDHIT; }
};

class fxExplosion : public fx
{
	fxOwner           _owner;
	GLvector2         _origin;
	GLrgba            _color_wave;
	GLrgba            _color_main;
	int               _begin;
	int               _frame;
	int               _duration;
	float             _size;
	float             _size_min;
	float             _size_max;
	bool              _flash;
	int               _damage;

	bool              Blast(GLvector2 victim, float victim_size, GLvector2* contact, GLvector2* shove, int* damage);
	void              DoDamage();

public:
	void              Init(fxOwner own, GLvector2 origin, int damage);
	void              Init(fxOwner own, GLvector2 origin, int damage, float radius);
	bool              Collide(GLvector2 pos, GLvector2* pushback);
	void              ColorSet(GLrgba color1, GLrgba color2);
	void              Render();
	void              Update();
	void              SizeSet(float size);
	fxType            Type() { return FX_EXPLOSION; }
};

enum AccessType
{
	ACCESS_NONE,
	ACCESS_CHECKPOINT,
};

class fxAccess : public fx
{
	GLvector2         _bob;
	bool              _used;
	GLuvFrame*        _uv;
	GLquad            _quad;
	GLrgba            _color;
	GLrgba            _color_dark;
	int               _frames;
	int               _message;
	AccessType        _access;
	const char*       _offer_message;

public:
	void              Activate();
	void              Render();
	void              Update();
	void              Init(GLvector2 position, AccessType at, int message = 0);
	fxType            Type() { return FX_ACCESS; }
};

class fxMessage : public fx
{
	unsigned          _render_list;
	int               _start;
	float             _fade;
	GLvector2         _offset;
	GLvector2         _scroll;

public:
	void              Render();
	void              Update();
	void              Init(GLvector2 position, const char* message, ...);
	fxType            Type() { return FX_MESSAGE; }
};

enum PickupType
{
	PICKUP_GUN,
};

class fxPickup : public fx
{
	PickupType        _type;
	int               _projectile_id;
	float             _size;
	float             _bob;
	float             _bob_cycle;
	bool              _bob_up;

	GLvector2         _sprite_position;
	float             _sprite_angle;
	float             _sprite_size;
	enum SpriteEntry  _sprite;
	GLrgba            _sprite_color;
	bool              _touching_player;
	bool              _can_grab;

	bool              TouchingPlayer();
	void              GiveGun();
	bool              PlayerAlreadyHasThis();
public:
	void              InitGun(GLvector2 position, int gun_id);
	void              Render();
	void              Update();
	fxType            Type() { return FX_POWERUP; }
};

class fxProjectile : public fx
{
	fxOwner           _owner;             //If this is owned by the player or robots.
	GLvector2         _vector;            //Normalized vector of travel.
	int               _bounces;           //Used to span child projhectiles as reflections of this one.
	int               _damage;            //Damage, as adjusted by player damage scaling if applicable.
	int	              _hits;              //How many things this bullet can hit before it dies. Decrements with each hit.
	const Projectile* _projectile;        //Stores all the info for this type of projectile.
	class bodyTail    _tail;              //A comet-tail that streams behind the object.
	bool              _has_tail;          //If the comet-tail is used.
	bool              _has_homing_lock;   //If we're currently tracking a target.
	bool              _is_armed;          //For homing: Have we flown far enough to be ready to explode?
	bool              _is_accelerating;   //For projectiles that accelerate. Nothing to do with gravity.
	bool              _is_disabled;       //If this projectile has been shot down.
	bool              _can_bounce;        //Can override the number of bounces.
	bool              _hit_someone;       //If the projectile connected with something that can take damage.
	float             _speed;             //Movement speed, distance per frame.
	float             _disabled_spin;     //The spin speed when this thing is shot down.
	float             _color_fade;        //The visual fadeout of the projectile.
	GLvector2         _beam_end;          //The impact point of the beam. Used for bouncing.
	GLvector2         _beam_bounce;       //If the beam needs to bounce, it does so on this vector.

	GLvector2         _movement;          //The total distance we move each frame.
	GLvector2         _movement_step;     //A single step for the purposes of collsion detection.
	int               _steps_per_frame;   //This, times _movement_step, should equal _movement.
	int               _frames;            //How many frames this object has been alive.
	int               _last_robot_hit;    //To avoid hitting the same robot more than once when passing through targets.
	int               _next_homing;       //Game tick where we can re-evaluate our homing trajectory.
	int               _next_spark;        //Game tick when we release our next particle effect.
	int               _tick_begin;
	int               _tick_end;

	float             _sprite_radius;     //For collision checking. How far from the origin can the sprite reach?
	GLvector2         _sprite_position;
	GLvector2         _sprite_size;
	float             _sprite_angle;
	GLrgba            _sprite_color;
	bool              _sprite_glow;

	void              BoltInit();
	void              BoltVector(GLvector2 new_vector);
	void              BoltUpdate();
	GLvector2         BoltTarget();
	void              BoltHoming();
	void              BoltEnd();
	void              BoltExplode();

	void              BeamInit();
	bool              BeamHit(GLvector2 pos, GLvector2* normal, bool* terminate, bool* hit_robot);
	void              BeamUpdate();

public:
	void              Init(fxOwner owner, const Projectile* type, GLvector2 pos, GLvector2 vector, int damage_level = 0);
	void              BouncesSet(int num) { _bounces = num; }
	void              Disable();
	bool              Disabled() { return _is_disabled; }
	GLvector2         Origin() { return _origin; }
	fxOwner           Owner() { return _owner; }
	void              Render();
	float             Radius() { return _sprite_radius; }
	bool              Shootable();
	void              Update();
	fxType            Type() { return FX_PROJECTILE; }
};

#define MAX_DUST            400

struct Mote
{
	GLvector2                 position;
	GLvector2                 drift;
	SpriteEntry								sprite;
	GLrgba										color;
	float                     depth;
	float                     angle;
	float                     spin;
	float                     size;
};

class fxDust
{
	Mote                      _mote[MAX_DUST];
	GLvector2                 _field;
	int                       _update;

public:
	void                      Init();
	void                      Render(GLrgba c);
	void                      Update();
};

class fxDevice : public fx
{
public:
	virtual void              RenderOccluded() = 0;
	virtual bool              Collide(GLvector2 pos) = 0;
	virtual void              Hit(GLvector2 pos, int damage) = 0;
};

enum DoorFacing
{
	DOOR_UP,
	DOOR_DOWN,
	DOOR_RIGHT,
	DOOR_LEFT,
};

class fxDoor : public fxDevice
{
	GLvector2         _move_direction;      //The direction the door will slide when opened.
	GLvector2         _current_position;    //Where the door is now, based on open / closed state.
	GLvector2         _collision_size;      //Dimensions of the door for collision purposes.
	GLvector2         _through;             //Direction player moves when going through door.
	DoorFacing        _facing;
	SpriteEntry       _sprite;
	Line2D            _line;                //Line used my visibility & shadow engaine for occlusion.
	int               _destination_zone;
	int               _open_timeout;
	float             _angle;
	float             _aperture;
	bool              _triggered;
	bool              _locked;

	void              Rebuff(GLvector2 direction);
	void              Trigger();
public:
	bool              Collide(GLvector2 pos);
	void              Hit(GLvector2 pos, int damage);
	void              Init(GLvector2 position, DoorFacing direction, SpriteEntry se, int destination, bool locked);
	Line2D            Line() { return _line; }
	void              Render();
	void              RenderOccluded();
	void              Update();
	fxType            Type() { return FX_DOOR; }
};

enum ForcefieldType
{
	FORCEFIELD_NONE,
	FORCEFIELD_BOSS,
	FORCEFIELD_MOBS,
};

class fxForcefield : public fx
{
	GLvector2         _position[4];
	GLvector2         _size;
	GLrgba            _color;
	ForcefieldType		_type;
	string            _sound_rebuff;
	int               _color_cycle;
	bool              _on;
	GLbbox2           _bbox;

public:
	void              Init(GLvector2 position, ForcefieldType type);
	void              Render();
	fxType            Type() { return FX_FORCEFIELD; }
	void              Update();
};

class fxHat : public fx
{
	SpriteEntry				_sprite;
	GLvector2					_movement;
	float							_spin;
	float							_size;
	float							_angle;
	bool							_worn;
	GLrgba						_color;

public:
	void							Init(SpriteEntry s, float size, GLrgba color);
	void							Wear(GLvector2 position, float angle);
	void							Drop(GLvector2 movement, float spin);
	void							Update();
	void							Render();
	float							Size() { return _size; }
	fxType            Type() { return FX_HAT; }
};

#endif // FX_H
