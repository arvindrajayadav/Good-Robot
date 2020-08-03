/*-----------------------------------------------------------------------------

  Fx.cpp

  Good Robot
  (c) 2013 Shamus Young

  A collection of small-time effects classes that aren't big enough to justify
  a module of their own.

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "bodyparts.h"
#include "camera.h"
#include "collision.h"
#include "entity.h"
#include "env.h"
#include "fx.h"
#include "game.h"
#include "hud.h"
#include "interface.h"
#include "menu.h"
#include "particle.h"
#include "player.h"
#include "projectile.h"
#include "random.h"
#include "render.h"
#include "robot.h"
#include "system.h"
#include "world.h"
#include "zone.h"

#define EXPLOSION_TIME          600 //milliseconds
#define EXPLOSION_MIN_SIZE      0.1f
#define EXPLOSION_MAX_SIZE      0.5f
#define EXPLOSION_FORCE         0.003f
#define EXPLOSION_DAMAGE        5     //An explosion takes a bullet damage and multiplies by this.
#define SHIELD_FLASH            1000 //milliseconds

#define POWERUP_LIFESPAN        60000 //base lifespan, in milliseconds
#define POWERUP_BOB_TIME        2333
#define POWERUP_BOB_HALF        (POWERUP_BOB_TIME/2)
#define POWERUP_WAVE_TIME       3500
#define POWERUP_WAVE_HALF       (POWERUP_WAVE_TIME/2)
#define POWERUP_GATHER_TIME     1000

#define MISSILE_SIZE            0.15f
#define MISSILE_HOMING_RANGE    5

#define MESSAGE_LENGTH          256
#define MESSAGE_TEXT_SIZE       0.007f
#define MESSAGE_LIFESPAN        2000
#define MESSAGE_DEPTH           0.3f

#define PICKUP_SIZE             0.2f

static int                      fx_id;

/*-----------------------------------------------------------------------------
Explosion class
-----------------------------------------------------------------------------*/

void fxExplosion::Init(fxOwner own, GLvector2 origin, int damage)
{
	_id = fx_id++;
	_begin = GameTick();
	_origin = origin;
	_active = true;
	_owner = own;
	_size_min = EXPLOSION_MIN_SIZE;
	_size_max = EXPLOSION_MAX_SIZE + 4;
	if (_owner == OWNER_PLAYER)
		_size_max *= Env().player_explosion_scale;
	_duration = EXPLOSION_TIME;
	_size = 1.0f;
	_frame = 0;
	_flash = false;
	_damage = damage;
	_color_main = GLrgba(1, 0.2f, 0);
	_color_wave = GLrgba(1, 0.6f, 0);
	if (_owner == OWNER_PLAYER)
		WorldSkyFlash(1);
	DoDamage();
	AudioPlay("explosion", _origin);
}

void fxExplosion::Init(fxOwner own, GLvector2 origin, int damage, float radius)
{
	_id = fx_id++;
	_begin = GameTick();
	_origin = origin;
	_active = true;
	_damage = damage;
	_owner = own;
	_size_min = radius / 2;
	_size_max = radius;
	_duration = EXPLOSION_TIME;
	_size = radius;
	_frame = 0;
	_flash = false;
	_color_main = GLrgba(1, 0.2f, 0);
	_color_wave = GLrgba(1, 0.6f, 0);
	DoDamage();
	AudioPlay("explosion", _origin);
}

void fxExplosion::SizeSet(float size)
{
	_size_min = EXPLOSION_MIN_SIZE * size;
	_size_max = EXPLOSION_MAX_SIZE * size;
	_duration = (int)(EXPLOSION_TIME * size);
	if (_owner == OWNER_PLAYER)
		_size_max *= Env().player_explosion_scale;
}

void fxExplosion::ColorSet(GLrgba color1, GLrgba color2)
{
	_color_main = color1;
	_color_wave = color2;
}

void fxExplosion::DoDamage()
{
	GLvector2 offset;
	Robot*      bot;

	//If the player made this, then deal damage to all robots in the blast radius.
	if (_owner != OWNER_ROBOTS) {
		for (int b = 0; b < EntityRobotCount(); b++) {
			bot = EntityRobot(b);
			if (bot->Dead())
				continue;
			if (bot->Invulnerable()) {
				AudioPlay("immune", bot->Position());
				continue;
			}
			offset = bot->Position() - _origin;
			if (offset.Length() < _size_max + bot->Size())
				bot->Damage((int)_damage);
		}
		if (_owner == OWNER_PLAYER)
			CameraShake(Env().screen_shake_explode);
	}
	if (_owner != OWNER_PLAYER) {
		GLvector2   shove;
		GLvector2   contact;
		int         dam;

		if (Blast(PlayerPosition(), PlayerSize(), &contact, &shove, &dam)) {
			PlayerDamage(dam);
		}
	}
}

void fxExplosion::Update()
{
	int         elapsed;
	GLvector2   shove;
	GLvector2   contact;
	int         dam;
	GLvector2   delta;

	if (!_active)
		return;
	elapsed = GameTick() - _begin;
	if (elapsed > _duration) {
		_active = false;
		return;
	}
	_frame = (_frame + 1) % 8;
	_flash = _frame == 0;
	if (_flash) {
		float     circum;
		int       points;
		int       step;
		int       angle_offset;
		GLvector2 pos;
		GLrgba    color;
		bool      flip = false;

		circum = PI * _size * _size;
		points = (int)(circum / (_size / 8));
		points = max(points, 1);
		step = 360 / points;
		angle_offset = RandomVal(360);
		for (int a = 0; a < 360; a += step) {
			float     size;
			color = flip ? _color_main : _color_wave;
			pos = SpriteMapVectorFromAngle(a + angle_offset);
			size = _size * (RandomFloat() + 0.5f) * 0.5f;
			ParticleBloom(_origin + pos*size, color, size, (EXPLOSION_TIME - elapsed) + RandomVal(100));
			if (flip && _frame > 4)
				ParticleSmoke(_origin + SpriteMapVectorFromAngle(a - angle_offset)*_size*RandomFloat(), _size, 1);
			flip = !flip;
		}
		ParticleSmoke(_origin, _size * 3, 1);
	}
	_size = (float)elapsed / _duration;
	_size = _size_min + _size * _size_max;
	//Have explosions shove the player.
	if (_owner != OWNER_PLAYER) {
		if (Blast(PlayerPosition(), PlayerSize(), &contact, &shove, &dam)) {
			PlayerShove(shove);
			if (_flash) {
				//PlayerDamage(dam);
				ParticleGlow(contact, shove, _color_main, _color_wave, 1, PlayerSize() / 2);
			}
		}
	}
}

void fxExplosion::Render()
{
	if (!_active)
		return;
	//RenderQuad (_origin, SPRITE_SHOCKWAVE, _color_wave, _size*2, 0, DEPTH_EXPLOSIONS, true);
	RenderQuad(_origin, SPRITE_GLOW, _color_main, _size * 2, 0, DEPTH_EXPLOSIONS, true);
	if (_flash)
		RenderQuad(_origin, SPRITE_GLOW, _color_main, _size_max, 0, DEPTH_EXPLOSIONS, true);
}

bool fxExplosion::Blast(GLvector2 victim, float victim_size, GLvector2* contact, GLvector2* shove, int* damage)
{
	GLvector2 offset;
	float     distance;
	float     intensity;

	if (!_active)
		return false;
	offset = victim - _origin;
	victim_size /= 2;
	//If they are out of range, ignore them.
	if (abs(offset.x) > _size || abs(offset.y) > _size)
		return false;
	distance = offset.Length();
	if (distance > _size + victim_size)
		return false;
	offset.Normalize();
	*contact = _origin + offset * distance;
	intensity = 1.0f - (distance / _size);
	offset.Normalize();
	*shove = offset * EXPLOSION_FORCE * intensity;
	*damage = (int)((float)_damage * intensity);
	*damage = max(*damage, 1);
	return true;
}

/*-----------------------------------------------------------------------------
Shield class - effect when player's shield is hit
-----------------------------------------------------------------------------*/

void fxShieldHit::Init(GLrgba color, int timestamp, float size)
{
	_id = fx_id++;
	_size = size;
	_angle = 0;
	_color = _start_color = color;
	_begin = timestamp;
	_active = true;
	_use_hit = false;
	_bright = 1;
	_quad = SpriteMapQuad(0);
}

void fxShieldHit::HitAngle(int angle)
{
	_angle = angle;
	_use_hit = true;
	_quad = SpriteMapQuad(_angle);
}

void fxShieldHit::Update()
{
	int         elapsed;

	_origin = PlayerPosition();
	if (!_active)
		return;
	elapsed = GameTick() - _begin;
	_bright = 1 - ((float)elapsed / SHIELD_FLASH);
	_color = _start_color * _bright;
	if (elapsed > SHIELD_FLASH)
		_active = false;
}

void fxShieldHit::Render()
{
	if (!_active)
		return;
	RenderQuad(_origin, SPRITE_ORB, _color, _size, (float)_angle, DEPTH_FX, true);
	RenderQuad(_origin, SPRITE_SHIELD, _color, _size, 180 + (float)_angle, DEPTH_FX, true);
}

/*-----------------------------------------------------------------------------
Powerup class
-----------------------------------------------------------------------------*/

void fxPowerup::Init(GLvector2 position, PowerupType type, int value)
{
	_id = fx_id++;
	if (type == POWERUP_COIN) {
		_color = GLrgba(0.5f, 0.99f, 0.2f);
		_color_particle1 = GLrgba(0.0f, 0.5f, 0.0f);
		_color_particle2 = GLrgba(0.00f, 0.3f, 0.2f);
		if (value >= 1000) //Thousands!
			_size = 0.3f;
		else if (value >= 100)  //Benjamins
			_size = 0.2f;
		else if (value >= 10) //Tens
			_size = 0.1f;
		else //Ones
			_size = 0.08f;
		_sprite = SPRITE_COIN;
	}
	else if (type == POWERUP_MISSILES) {
		_color_particle1 = GLrgba(0.1f, 0.1f, 0.1f);
		_color_particle2 = GLrgba(0.1f, 0.1f, 0.1f);
		_size = 0.10f;
		_color = GLrgba(0, 0, 0);
		_sprite = SPRITE_MISSILE;
	}
	else { //a shield
		_size = 0.2f;
		_color_particle1 = GLrgba(0.0f, 0.1f, 0.2f);
		_color_particle2 = GLrgba(0.0f, 0.0f, 0.2f);
		_color = GLrgba(0.5f, 0.3f, 0.9f);
		_sprite = SPRITE_ORB;
	}
	//Powerups last for base lifespan, plus one second for each point of value they have.
	_lifespan = POWERUP_LIFESPAN + value * 1000;
	_type = type;
	_value = value;
	_begin = GameTick();
	_animate_offset = RandomVal(99999);
	_origin = position;
	_active = true;
	_gathering = false;
	_phase = 0;
	_angle = 0;
	_particle_cooldown = 0;
	_momentum = GLvector2(RandomFloat() - 0.5f, RandomFloat() - 0.5f) * 0.1f;
};

void fxPowerup::Update()
{
	int         elapsed;
	GLvector2   offset;
	int         animate;

	if (!_active)
		return;
	elapsed = GameTick() - _begin;
	if (elapsed > POWERUP_LIFESPAN && !_gathering) {
		_active = false;
		ParticleSparks(Position(), _color, 3);
		return;
	}
	if (GameTick() > _particle_cooldown) {
		_particle_cooldown = GameTick() + 250;
		//ParticleGlow(_origin, GLvector2(), _color_particle1, _color_particle2, 1, _size);
	}
	//Animate the coin bobbing up and down.
	animate = GameTick() + _animate_offset;
	animate %= POWERUP_BOB_TIME;
	if (animate > POWERUP_BOB_HALF)
		animate = POWERUP_BOB_TIME - animate;
	_phase = ((float)animate - POWERUP_BOB_HALF) / POWERUP_BOB_HALF;
	//Animate the coin turning clockwise / ccw
	animate = GameTick() + _animate_offset;
	animate %= POWERUP_WAVE_TIME;
	if (animate > POWERUP_WAVE_HALF)
		animate = POWERUP_WAVE_TIME - animate;
	if (_gathering && PlayerIgnore()) {
		_gathering = false;
		_begin = GameTick();
	}
	if (_gathering) {
		_angle++;
		_angle2 = _angle;
	}
	else {
		if (_type == POWERUP_SHIELD) {
			_angle = 175 + animate / 160;
			_angle2 = 175 - animate / 160;
		}
		else
			_angle = animate / 50;
	}
	offset = PlayerPosition() - _origin;
	if (_gathering) {
		if (GameTick() > _force_gather || offset.Length() < _size) {
			if (_type == POWERUP_COIN)
				PlayerAddXp(_value);
			_active = false;
			return;
		}
		offset *= 0.1f;
		_origin += offset;
	}
	else { //slide until we stop or are picked up.
		float   reach;

		reach = PlayerGatherDistance() + Env().pickup_range;
		if (elapsed > 1000 && !PlayerIgnore() && abs(offset.x) < reach && abs(offset.y) < reach) {
			_gathering = true;
			_force_gather = GameTick() + POWERUP_GATHER_TIME;
		}
		if (Collision(_origin + _momentum, _size))
			_momentum = GLvector2();
		_momentum *= 0.93f;
		_origin += _momentum;
	}
}

void fxPowerup::Render()
{
	if (!_active)
		return;
	if (_type == POWERUP_COIN) {
		RenderQuad(_origin + GLvector2(0, _phase*0.1f), _sprite, _color, _size, (float)_angle, DEPTH_FX, false);
	}
	else if (_type == POWERUP_MISSILES)
		RenderQuad(_origin + GLvector2(0, _phase*0.1f), _sprite, _color, _size, (float)_angle, DEPTH_FX, false);
	else //render shield thing
		RenderQuad(_origin + GLvector2(0, _phase*0.1f), _sprite, _color, _size, 0, DEPTH_FX, true);
}

/*-----------------------------------------------------------------------------
Access class - In-world markers that the player can interact with to open menus.
-----------------------------------------------------------------------------*/
#if 0
void fxAccess::Init(GLvector2 position, AccessType at, int message)
{
	_id = fx_id++;
	_origin = position;
	_quad = SpriteMapQuad(180);
	_access = at;
	_active = true;
	_frames = 0;
	_message = message;
	_used = false;
	if (_access == ACCESS_CHECKPOINT) {
		_color = GLrgba(0.2f, 1, 0.2f) * 0.66f;
		_color_dark = GLrgba(0, 1, 0) * 0.33f;
		_uv = SpriteMapLookup(SPRITE_PICKUP, SPRITE_PICKUP_CHECKPOINT);
		_offer_message = "";
	}
	for (unsigned i = 0; i < 4; i++) {
		_quad.corner[i] *= ACCESS_SIZE;
		_quad.corner[i] += _origin;
	}
}

void fxAccess::Activate()
{
}

void fxAccess::Update()
{
	GLvector2     player;
	GLvector2     offset;

	_frames = (_frames + 1) % ACCESS_SPARK_INTERVAL;
	_bob = GLvector2();
	player = PlayerPosition();
	offset = player - _origin;
	if (abs(offset.x) < 1.0f && abs(offset.y) < 1.0f && _access != ACCESS_CHECKPOINT) {
		if (PlayerAccessActivated())
			Activate();
		PlayerOfferAccess(offset.Length(), _offer_message);
	}
	if (_access == ACCESS_CHECKPOINT) {
		_color = Player()->Color();
		_color_dark = _color * 0.2f;
		if (!_frames) {
			ParticleSprite(_origin + GLvector2(0, 0.5f), GLvector2(0, -0.02f), _color, SPRITE_LANDING, 1, 0.4f);
			ParticleGlow(_origin + GLvector2(0, 0.5f), GLvector2(0, -0.02f), _color*0.5f, _color_dark*0.5f, 1);
		}
	}
	else { //story
		if (!_frames) {
			if (abs(offset.x) < 4 && abs(offset.y) < 4)
				ParticleGlow(_bob + _origin, GLvector2(), _color*0.2f, _color_dark*0.2f, 1);
			if (abs(offset.x) < 2 && abs(offset.y) < 2)
				ParticleGlow(_bob + _origin, offset * 0.03f, _color_dark*0.1f, _color_dark*0.1f, 3);
		}
	}
}

void fxAccess::Render()
{
	if (_access == ACCESS_CHECKPOINT)
		RenderQuad(_origin + GLvector2(0, 0.33f), SPRITE_LANDING, _color, 0.5f, 0, 0.1f, false);
	else
		RenderQuad(_origin + _bob, SPRITE_STORY, _color, 0.6f, 0, 0.1f, true);
}
#endif

/*-----------------------------------------------------------------------------
Message class - words that appear on-screen and then fade away.
-----------------------------------------------------------------------------*/

void fxMessage::Init(GLvector2 position, const char* message, ...)
{
	static char       msg_text[MESSAGE_LENGTH];
	va_list           marker;
	const Font*       f;
	vector<FontChar>  parsed;

	va_start(marker, message);
	vsprintf(msg_text, message, marker);
	va_end(marker);

	_id = fx_id++;
	_origin = position;
	_start = GameTick();
	_fade = 0;
	_active = true;

	f = InterfaceFont(FONT_HUD);

	parsed = f->Parse(msg_text);
	_offset.x = (float)f->Width(parsed) / 2;
	_offset.y = (float)f->Height() / 2;
	_render_list = RenderListCompile(_id);
	f->Print(GLcoord2(), parsed);
	RenderListEnd();
}

void fxMessage::Update()
{
	int     elapsed;

	elapsed = GameTick() - _start;
	_fade = (float)elapsed / MESSAGE_LIFESPAN;
	_fade = pow(_fade, 3.0f);
	_fade = 1.0f - _fade;
	_scroll = _origin + GLvector2(0, -0.5f + 0.5f * _fade);
	if (elapsed > MESSAGE_LIFESPAN)
		_active = false;
}

void fxMessage::Render()
{
	int     prev_texture;

	if (!_active)
		return;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_texture);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glPushMatrix();
	glTranslatef(_scroll.x, _scroll.y, MESSAGE_DEPTH);
	glScalef(MESSAGE_TEXT_SIZE * _fade, MESSAGE_TEXT_SIZE, MESSAGE_TEXT_SIZE);
	glTranslatef(-_offset.x, -_offset.y, -MESSAGE_DEPTH);
	RenderListCall(_id, _render_list);
	//RenderQuad (_origin, SPRITE_SHOCKWAVE, GLrgba (1,1,1), 1, 0, MESSAGE_DEPTH, true);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glBindTexture(GL_TEXTURE_2D, prev_texture);
}

/*-----------------------------------------------------------------------------
Projectile class - intended to supersede and unify old missile/bullet/beam types.
-----------------------------------------------------------------------------*/

#define PROJECTILE_MAX_STEP 0.08

void fxProjectile::Init(fxOwner owner, const Projectile* type, GLvector2 pos, GLvector2 vector, int damage_level)
{
	_owner = owner;
	_active = true;
	_projectile = type;
	_origin = pos;
	_vector = vector;
	_frames = 0;
	_last_robot_hit = 0;
	_hits = type->_penetration;
	_has_tail = false;
	_has_homing_lock = false;
	_is_disabled = false;
	_is_armed = true;
	_is_accelerating = false;
	_can_bounce = true;
	_hit_someone = false;
	_bounces = 0;
	_next_homing = 0;
	_next_spark = 0;
	_tick_begin = GameTick();
	_tick_end = _tick_begin + _projectile->_tick_lifespan;
	_color_fade = 1.0f;
	if (_projectile->_has_acceleration) {
		_is_accelerating = true;
		_speed = _projectile->_start_speed;
	}
	else
		_speed = _projectile->_speed;
	_damage = (int)((float)_projectile->_damage * EnvDamageScaling(damage_level).val);

	//If these projectiles are intended to scatter a bit...
	if (_projectile->_scatter > 0.0f) {
		_vector += GLvector2(RandomFloat() - 0.5f, RandomFloat() - 0.5f) * _projectile->_scatter;
		float speed_variation = 1.0f + (RandomFloat() - 0.5f) * _projectile->_scatter;
		_speed = _projectile->_speed * speed_variation;
		_speed = max(_speed, (_projectile->_speed / 3.0f));
	}
	_sprite_position = _origin;
	_sprite_angle = _vector.Angle();
	_sprite_size = _projectile->_size;
	_sprite_radius = _sprite_size.Length();
	_sprite_color = _projectile->_color;
	_sprite_glow = _projectile->_is_glowing;
	if (_projectile->_type == PROJECTILE_BOLT)
		BoltInit();
	if (_projectile->_type == PROJECTILE_BEAM)
		BeamInit();
}

bool fxProjectile::Shootable() { return _projectile->_is_shootable; }

void fxProjectile::Disable()
{
	_is_disabled = true;
	if (_projectile->_is_volatile) {
		fxExplosion*    e;
		e = new fxExplosion;
		fxOwner   new_owner;

		new_owner = _owner;
		if (_projectile->_friendly_fire)
			new_owner = OWNER_NONE;
		e->Init(new_owner, _origin, 0, _projectile->_size.x * 2.0f);
		e->ColorSet(_projectile->_color, _projectile->_aura_color);
		EntityFxAdd(e);
		ParticleSparks(_origin, _projectile->_color, 8);
	}
}

void fxProjectile::BoltInit()
{
	BoltVector(_vector);
	if (_projectile->_tail_brightness > 0) {
		float size = min(_projectile->_size.x, _projectile->_size.y) / 2;
		_tail.Init(_origin, _sprite_color * _projectile->_tail_brightness, size);
		_has_tail = true;
	}
	if (Shootable())
		_disabled_spin = ((RandomFloat() - 0.5f) * 20.0f) + ((RandomFloat() - 0.5f) * 20.0f);
}

void fxProjectile::BoltVector(GLvector2 vector)
{
	//Set up our per-frame movement speed.
	_movement = vector.Normalized() * _speed;
	_movement_step = _movement;
	_steps_per_frame = 1;
	//At high speeds, the projectile might skip over a target.
	//So if the steps are too big, we break them down into sub-steps until
	//we're sure we won't miss anything.
	while (_movement_step.Length() > PROJECTILE_MAX_STEP) {
		_movement_step *= 0.5f;
		_steps_per_frame *= 2;
	}
}

void fxProjectile::BoltUpdate()
{
	GLvector2   old_origin;
	bool        speed_changed;

	old_origin = _origin;
	speed_changed = false;
	//If our lifespan is over, disable the missile as if it was shot down
	//and give it enough lifespan to tumble to earth.
	if (GameTick() >= _tick_end && !_projectile->_is_glowing) {
		_is_disabled = true;
		_tick_end += 5000;
	}
	if (_is_accelerating) {
		float   delta = (float)(GameTick() - _tick_begin) / (float)_projectile->_tick_accelerate;
		delta *= delta;
		if (delta >= 1.0f)
			_is_accelerating = false;
		_speed = syMathInterpolate(delta, _projectile->_start_speed, _projectile->_speed);
		_speed = clamp(_speed, _projectile->_start_speed, _projectile->_speed);
		speed_changed = true;
		_movement = _vector * _speed;
	}
	if (!_is_disabled && _projectile->_spark_interval > 0 && GameTick() > _next_spark) {
		_next_spark = GameTick() + _projectile->_spark_interval;
		ParticleSparks(_sprite_position, _movement * 0.5f, _sprite_color, 1);
	}
	//See if we need to fade out at the end of our lives.
	if (_projectile->_tick_fadeout > 0) { //Don't divide by zero.
		int   fade_start = _tick_end - _projectile->_tick_fadeout;
		_color_fade = (float)(GameTick() - fade_start) / (float)_projectile->_tick_fadeout;
		_color_fade = 1.0f - clamp(_color_fade, 0.2f, 1.0f);
		_color_fade *= _color_fade;
	}
	if (_is_disabled) {
		_movement.y += GRAVITY;
		speed_changed = true;
		_sprite_angle += _disabled_spin;
		_origin += _movement;
		_sprite_position = _origin;
		if (Collision(_origin)) {
			_active = false;
			AudioPlay("collide", _origin);
			ParticleDebris(_origin, 0.1f, _projectile->_debris, _projectile->_speed, _movement*-0.5f);
		}
		return;
	}
	_sprite_angle += _projectile->_spin;
	if (_projectile->_is_homing)
		BoltHoming();
	if (!_projectile->_is_weightless) {
		_movement.y += GRAVITY*_projectile->_gravity * 10.0f;
		speed_changed = true;
	}
	if (speed_changed) {
		_movement_step = _movement;
		_steps_per_frame = 1;
		//At high speeds, the projectile might skip over a target.
		//So if the steps are too big, we break them down into sub-steps until
		//we're sure we won't miss anything.
		while (_movement_step.Length() > PROJECTILE_MAX_STEP) {
			_movement_step *= 0.5f;
			_steps_per_frame *= 2;
		}
	}
	//Break our movement into small steps.
	for (int i = 0; i < _steps_per_frame; i++) {
		_origin += _movement_step;
		//If this isn't a robot bullet, see if it hit a robot
		if (_owner != OWNER_ROBOTS) {
			Robot*      bot;
			GLvector2   delta;

			for (int b = 0; b < EntityRobotCount(); b++) {
				bool  take_damage;

				bot = EntityRobot(b);
				if (bot->Dead())
					continue;
				if (bot->Id() == _last_robot_hit)
					continue;
				if (bot->Hit(_origin, take_damage)) {
					if (take_damage) {
						int damage;

						damage = _damage;
						if (EnvValueb(ENV_KFA))
							damage *= 50;
						bot->Damage(damage, _origin, _movement);
						//Bigger bots should spew more blood.
						int splats = (int)(bot->Size() * 10.0f);
						splats = clamp(splats, 2, 7);
						ParticleBlood(_origin, bot->BodyColor(), bot->Size() / 4.0f, splats, 0.1f, _movement_step.Normalized());
						ParticleDebris(_origin, 0.1f, 3, _projectile->_speed, _movement*0.5f);
						_last_robot_hit = bot->Id();
						_hit_someone = true;
						AudioPlay("hit_robot", _origin);
					}
					else { //We hit a robot that's invulnerable in this location.
						ParticleSparks(_origin, _sprite_color, 2);
						AudioPlay("immune", _origin);
					}
					_hits--;
					if (_hits <= 0)
						BoltEnd();
					break;
				}
			}
		}
	}
	//See if we hit a door.
	if (_owner != OWNER_ROBOTS) {
		for (unsigned i = 0; i < EntityDeviceCount(); i++) {
			fxDevice* device = EntityDeviceFromId(i);
			if (device->Collide(_origin)) {
				device->Hit(_origin, _damage);
				ParticleSparks(_origin, _sprite_color, 2);
				AudioPlay("hit", _origin);
				BoltEnd();
			}
		}
	}

	//If this isn't a player bullet, it can hit the player.
	if (_owner != OWNER_PLAYER && !PlayerIgnore()) {
		if (PlayerCollide(_origin)) {
			_hit_someone = true;
			PlayerDamage(_damage, _vector);
			ParticleSparks(_origin, _sprite_color, 5);
			BoltEnd();
		}
	}
	//See if we shot down any "missiles". Only player shots can do this.
	if (_owner == OWNER_PLAYER) {
		fxProjectile*   p = EntityProjectileList();
		for (int i = 0; i < MAX_PROJECTILES; i++) {
			if (!p[i].Active() || p[i].Disabled())
				continue;
			if (!p[i].Shootable()) //Don't try to shoot non-shootable things.
				continue;
			if (p[i].Owner() == _owner) //Please don't shoot down friendly missiles.
				continue;
			GLvector2 delta = _origin - p[i].Origin();

			//Discard if it's not in the missile's bounding box.
			if (abs(delta.x) > p[i].Radius() || abs(delta.y) > p[i].Radius())
				continue;
			if (delta.Length() < p[i].Radius()) {
				//If the player shot down a projectile...
				if (_owner == OWNER_PLAYER)
					Player()->TriviaModify(TRIVIA_MISSILES_DESTROYED, 1);
				AudioPlay("hit", _origin);
				p[i].Disable();
				ParticleDebris(_origin, 0.1f, 3, _projectile->_speed, _movement*0.25f);

				_hits--;
				if (_hits <= 0)
				{
					BoltEnd();
					_active = false;
				}
				return;
			}
		}
	}
	//Walls are big and don't require the fine-grain checks above.
	GLvector2 wall;
	if (Collision(_origin, &wall, NULL)) {
		_origin = old_origin;
		ParticleDebris(_origin, 0.1f, _projectile->_debris, _projectile->_speed, _movement*-0.5f);
		if (_bounces) {
			fxProjectile p;
			fxOwner   new_owner;

			new_owner = _owner;
			if (_projectile->_friendly_fire)
				new_owner = OWNER_NONE;
			p.Init(new_owner, _projectile, _origin, GLreflect2(_vector.Normalized(), wall.Normalized()));
			p.BouncesSet(_bounces - 1);
			EntityProjectileAdd(p);
		}
		BoltEnd();
	}
	//if ((_frames % 7) == 0)
	//ParticleGlow (_origin, _movement * 0.66f, _sprite_color*0.5f, _sprite_color*0.1f, 1, 0.7f);
	if (_has_tail) {
		_tail.ColorSet(GLrgba(_sprite_color) * _projectile->_tail_brightness);
		_tail.Update(_origin, GameTick());
	}
	_sprite_position = _origin;
}

//Used by homing projectiles. Examine the playfield and find a target to lock onto.
GLvector2 fxProjectile::BoltTarget()
{
	GLvector2 nothing;

	//This is where we will go if there's nothing to lock onto. Just aim straight ahead.
	nothing = _origin + _vector;
	if (_owner != OWNER_PLAYER) {
		if (PlayerIgnore())
			return nothing;
		if (_is_armed)
			return PlayerPosition();
		return nothing;
	}

	if (!EntityRobotCount())
		return nothing;
	//Player owns this missile, so target robots...
	Robot*    bot;
	Robot*    best;
	GLvector2 offset;
	float     closest;
	float     distance;
	float     diff;
	float     angle;
	float     angle_current;

	best = NULL;
	closest = 99999.9f;
	angle_current = _vector.Angle();
	for (int b = 0; b < EntityRobotCount(); b++) {
		bot = EntityRobot(b);
		offset = bot->Position() - _origin;
		//Do a quick distance check to see if this bot is too far away for a lock-on.
		if (abs(offset.x) > MISSILE_HOMING_RANGE)
			continue;
		if (abs(offset.y) > MISSILE_HOMING_RANGE)
			continue;
		//Fo a more precise distance check.
		distance = offset.Length();
		if (distance > closest)
			continue;
		//See if this is within the narrow cone in front of the missile.
		angle = offset.Angle();
		diff = syMathAngleDifference(angle_current, angle);
		if (diff > 45)
			continue;
		//Make sure we have LOS to the robot.
		if (!CollisionLos(_origin, bot->Position(), 0.2f))
			continue;
		closest = distance;
		best = bot;
	}
	if (!best)
		return nothing;
	return best->Position();;
}

void fxProjectile::BoltHoming()
{
	GLvector2   target;
	GLvector2   delta;
	float       desired_angle;
	float       current_angle;
	float       turn;
	float       turn_rate;

	if (PlayerIgnore())
		return;
	//Find the best target, given our heading and distance.
	target = BoltTarget();
	//Make sure we have
	_has_homing_lock = CollisionLos(_origin, target, 0.2f);
	if (_has_homing_lock && _owner != OWNER_PLAYER)
		PlayerIncomingMissile(_origin);
	if (GameTick() > _next_homing) {
		//We adjust 10x per second.
		_next_homing = GameTick() + 100;
		if (_has_homing_lock) {
			delta = target - _origin;
			current_angle = _movement.Angle();
			desired_angle = delta.Angle();
			//Turn rate is in degrees per second. Since we do this 10x
			//a second, we should do 1/10th of the turn.
			turn_rate = _projectile->_turn_rate / 10.0f;
			turn = syMathAngleDifference(desired_angle, (float)current_angle);
			turn = clamp(turn, -turn_rate, turn_rate);
			current_angle += (int)turn;
			_vector = GLvectorFromAngle((float)current_angle);
			BoltVector(_vector);
			//Only turn the sprite to face the new heading if we're NOT a spinning projectile.
			if (_projectile->_spin == 0.0f)
				_sprite_angle = current_angle;
		}
	}
}

void fxProjectile::BoltEnd()
{
	_active = false;
	AudioPlay("hit", _origin);
	ParticleBloom(_origin, _sprite_color, _sprite_size.y, 500);
	if (_projectile->_explosion_radius > 0.0f) {
		BoltExplode();
	}
	//Did the player dodge this homing missile?
	if (_owner == OWNER_ROBOTS && _projectile->_is_homing && !_hit_someone)
		Player()->TriviaModify(TRIVIA_MISSILES_EVADED, 1);
}

void fxProjectile::BoltExplode()
{
	fxExplosion*  e;
	int           player_health_before;
	int           player_health_after;

	player_health_before = Player()->Shields();
	if (_is_armed) {
		e = new fxExplosion;
		fxOwner   new_owner;
		float     radius;

		new_owner = _owner;
		//Prevent div by 0.
		radius = max(_projectile->_explosion_radius, 0.05f);
		if (_projectile->_friendly_fire)
			new_owner = OWNER_NONE;
		e->Init(new_owner, _origin, _damage, radius);
		e->ColorSet(_projectile->_color, _projectile->_aura_color);
		EntityFxAdd(e);
		//Shake the camera based on the distance to this explosion.
		float distance_to_camera = GLvector2(_origin - CameraPosition2D()).Length();
		float magnitude = 1 - (distance_to_camera / radius);
		magnitude = clamp(magnitude, 0.0f, 1.0f);
		magnitude = magnitude*magnitude;
		CameraShake(magnitude * _projectile->_screen_shake);
	}
	else {
		AudioPlay("missile_crash", _origin);
		ParticleDebris(_origin, MISSILE_SIZE / 3, 6);
	}
	player_health_after = Player()->Shields();
	//If the missile is not owned, then it was a robot missile that went
	//"live" when passing close to the player. If we didn't manage to damage
	//the player with the explosion we just initialized, then this missile was
	//close to the player but ultimately missed. Give credit for evading it.
	if (_owner == OWNER_NONE && player_health_after == player_health_before && _projectile->_is_homing)
		Player()->TriviaModify(TRIVIA_MISSILES_EVADED, 1);
	_active = false;
}

void fxProjectile::Update()
{
	if (!_active)
		return;
	if (_projectile->_type == PROJECTILE_BOLT)
		BoltUpdate();
	if (_projectile->_type == PROJECTILE_BEAM)
		BeamUpdate();
	if (GameTick() > _tick_end)
		_active = false;
	_sprite_color = GLrgba(_projectile->_color) * _color_fade;
	_frames++;
}

void fxProjectile::Render()
{
	if (!_active)
		return;
	RenderQuad(_sprite_position, _projectile->_sprite, _sprite_color, _sprite_size, 180.0f + _sprite_angle, DEPTH_FX, _sprite_glow);
	if (_is_disabled)
		return;
	if (_projectile->_has_aura)
		RenderQuad(_sprite_position, SPRITE_GLOW, _projectile->_aura_color, _projectile->_aura_size, 0.0f, DEPTH_FX, true);
	if (_has_tail)
		_tail.Render();
}

//Since beams are instant hitscan damage, this init runs the entire course of the projectile's path.
void fxProjectile::BeamInit()
{
	GLvector2   step;
	GLvector2   scan;
	GLvector2   wall_normal;
	GLvector2   from_camera;
	bool        terminate;
	bool				hit_robot;
	float       max_range;

	max_range = CameraPosition().z * 2.0f;
	_beam_bounce = GLvector2(0, 1);
	_tick_end = _tick_begin + _projectile->_tick_fadeout;
	//Starting from our origin, trace our path until we hit a wall.
	step = _vector * 0.1f;
	scan = _beam_end = _origin;
	wall_normal = GLvector2(0, 1);
	for (int i = 0; i < 1000; i++) {
		scan += step;
		//Create a trail of fluffy glowing particles along the path of the beam.
		ParticleBloom(scan, _sprite_color, 0.33f, _projectile->_tick_lifespan * 2);
		//Don't let the beam go too far off-screen.
		from_camera = scan - CameraPosition2D();
		if (abs(from_camera.x) > max_range || abs(from_camera.y) > max_range) {
			_can_bounce = false;
			break;
		}
		//See if we hit something.
		if (BeamHit(scan, &wall_normal, &terminate, &hit_robot)) {
			if (terminate)
				_can_bounce = false;
			_beam_bounce = GLreflect2(_vector.Normalized(), wall_normal.Normalized());
			break;
		}
		if (hit_robot) {
			_hits--;
			if (_hits < 1) {
				terminate = true;
				_can_bounce = false;
				break;
			}
		}
		_beam_end = scan;
	}
	ParticleGlow(scan, GLvector2(), _sprite_color, _projectile->_aura_color, 2);
	_sprite_position = (_origin + scan) / 2;
	_sprite_angle = 90.0f + GLvector2(_origin - scan).Angle();
	_sprite_size.y = _projectile->_size.x;
	_sprite_size.x = GLvector2(_origin - scan).Length();
	_sprite_radius = _sprite_size.Length();
}

void fxProjectile::BeamUpdate()
{
	if (GameTick() > _tick_end)
		_active = false;
	if (_projectile->_tick_fadeout > 0) { //Don't divide by zero.
		_color_fade = (float)(_tick_end - GameTick()) / (float)_projectile->_tick_fadeout;
		_color_fade *= _color_fade;
	}
	_sprite_size.y = _projectile->_size.x * _color_fade;
	//If this beam needs to bounce, use the endpoint as the startingpoint for a new one.
	if (_frames == 10 && _can_bounce && _bounces > 0) {
		fxProjectile p;
		fxOwner   new_owner;

		new_owner = _owner;
		if (_projectile->_friendly_fire)
			new_owner = OWNER_NONE;
		p.Init(new_owner, _projectile, _beam_end, _beam_bounce);
		p.BouncesSet(_bounces - 1);
		EntityProjectileAdd(p);
	}
}

bool fxProjectile::BeamHit(GLvector2 pos, GLvector2* normal, bool* terminate, bool* hit_robot)
{
	float       ignore;

	//We set this to true if we want the beam to end HERE, with no further bouncing.
	//Basically, it's for when we hit a target that should absorb rather than reflect.
	*terminate = false;
	*hit_robot = false;
	if (Collision(pos, normal, &ignore, 0.1f))
		return true;
	//If this isn't a robot beam, see if it hit a robot
	if (_owner != OWNER_ROBOTS) {
		Robot*      bot;
		GLvector2   delta;

		for (int b = 0; b < EntityRobotCount(); b++) {
			bool  take_damage;

			bot = EntityRobot(b);
			if (bot->Dead())
				continue;
			//As we pass through a given robot, we should only deal damage to it ONCE.
			if (bot->Id() == _last_robot_hit)
				continue;
			if (bot->Hit(pos, take_damage)) {
				AudioPlay("hit", pos);
				if (take_damage) {
					bot->Damage(_damage, pos, _vector * 0.1f);
					ParticleSparks(pos, _sprite_color, 5);
					ParticleGlow(pos, GLvector2(), _sprite_color, _projectile->_aura_color, 2, _projectile->_size.x / 2);
					ParticleGlow(pos, GLvector2(), _sprite_color, _projectile->_aura_color, 2, bot->Size());
					_last_robot_hit = bot->Id();
					*hit_robot = true;
					if (bot->HasWeakpoints())
						*terminate = true;
				}
				//Beams can't go through foes with specific weak points. Looks odd.
				if (bot->HasWeakpoints()) {
					*normal = GLvector2(pos - bot->Position()).Normalized();
					return true;
				}
				break;
			}
		} //Done checking robots
		//Beams can hit machines and doors.
		for (unsigned i = 0; i < EntityDeviceCount(); i++) {
			fxDevice* device = EntityDeviceFromId(i);
			if (device->Collide(pos)) {
				device->Hit(pos, _damage);
				ParticleSparks(pos, _sprite_color, 2);
				AudioPlay("hit", pos);
				break;
			}
		}
	}
	//If this isn't a player beam, see if it hit the player
	if (_owner != OWNER_PLAYER && !PlayerIgnore()) {
		if (PlayerCollide(pos)) {
			PlayerDamage(_damage, _vector);
			ParticleSparks(_origin, _sprite_color, 5);
			ParticleGlow(pos, GLvector2(), _sprite_color, _projectile->_aura_color, 2, _projectile->_size.x / 2);
			ParticleGlow(pos, GLvector2(), _sprite_color, _projectile->_aura_color, 2, PlayerSize());
			return true;
		}
	}
	return false;
}

/*-----------------------------------------------------------------------------
Door class - moving doors.
-----------------------------------------------------------------------------*/

#define DOOR_SPEED        0.08f
#define DOOR_REBUFF       0.025f
#define DOOR_HEIGHT       1.1f
#define DOOR_HALF_HEIGHT  (DOOR_HEIGHT/2)
#define DOOR_THICKNESS    0.4f
#define DOOR_OPEN_TIME    2000

void fxDoor::Init(GLvector2 position, DoorFacing direction, SpriteEntry sprite_in, int zone_id, bool locked)
{
	_origin = position;
	_angle = 0;
	_active = true;
	_triggered = false;
	_facing = direction;
	_open_timeout = 0;
	_locked = locked;
	_aperture = 0.0f;
	_destination_zone = zone_id;
	if (direction == DOOR_UP || direction == DOOR_DOWN) {
		_move_direction = GLvector2(1, 0);
		_collision_size = GLvector2(DOOR_HEIGHT, DOOR_THICKNESS);
		_through = (direction == DOOR_UP) ? GLvector2(0, 1) : GLvector2(0, -1);
		_angle = 90.0f;
	}
	else {
		_move_direction = GLvector2(0, -1);
		_collision_size = GLvector2(DOOR_THICKNESS, DOOR_HEIGHT);
		_through = (direction == DOOR_RIGHT) ? GLvector2(-1, 0) : GLvector2(1, 0);
		_angle = 0.0f;
	}
	_sprite = sprite_in;
}

void fxDoor::Render()
{
	RenderQuad(_current_position, SPRITE_DOOR, GLrgba(1, 1, 1), DOOR_HEIGHT, _angle, DEPTH_DOORS, false);
	RenderQuad(_current_position, _sprite, GLrgba(1, 1, 1), DOOR_HEIGHT, 0, DEPTH_DOORS, false);
}

void fxDoor::RenderOccluded()
{
	RenderQuad(_current_position, SPRITE_DOOR, GLrgba(), DOOR_HEIGHT, _angle, DEPTH_DOORS, false);
	RenderQuad(_current_position, _sprite, GLrgba(1, 1, 1), DOOR_HEIGHT, 0, DEPTH_DOORS, false);
};

//When the player slams into a door.
void fxDoor::Rebuff(GLvector2 direction)
{
	GLvector2   player = PlayerOrigin();

	if (_facing == DOOR_UP)
		player.y = min(player.y, _current_position.y - _collision_size.y / 1.9f);
	if (_facing == DOOR_DOWN)
		player.y = max(player.y, _current_position.y + _collision_size.y / 1.9f);
	if (_facing == DOOR_RIGHT)
		player.x = max(player.x, _current_position.x + _collision_size.x / 1.9f);
	if (_facing == DOOR_LEFT)
		player.x = min(player.x, _current_position.x - _collision_size.x / 1.9f);

	PlayerOriginSet(player);
	PlayerShove(direction * DOOR_REBUFF);
	AudioPlay("collide", _origin);
}

//When the player enters the door, triggering a level change.
void fxDoor::Trigger()
{
	if (PlayerIgnore())
		return;
	WorldZoneTransition(_destination_zone);
	_triggered = true;
	PlayerAutoDrive(_through);
}

bool fxDoor::Collide(GLvector2 pos)
{
	GLvector2 delta = pos - _current_position;

	if (abs(delta.x) < _collision_size.x / 2 && abs(delta.y) < _collision_size.y / 2)
		return true;
	return false;
}

void fxDoor::Hit(GLvector2 pos, int damage)
{
	_open_timeout = GameTick() + DOOR_OPEN_TIME;
}

void fxDoor::Update()
{
	bool        open, closed, knock_knock, player_near;
	GLvector2   offset;
	GLvector2   player;

	open = _aperture == 1.0f;
	closed = _aperture == 0.0f;
	//knock_knock is true if the door should be open for any reason.
	knock_knock = false;
	player_near = false;
	player = PlayerPosition();
	_current_position = _origin + _move_direction * _aperture;
	offset = player - _origin;
	if (abs(offset.x) < 1 && abs(offset.y) < 1) {
		player_near = true;
		knock_knock = true;
	}
	if (_open_timeout > 0) {
		knock_knock = true;
		if (_open_timeout < GameTick())
			_open_timeout = 0;
	}
	if (!_locked) {
		if (knock_knock)
			_aperture += DOOR_SPEED;
		else
			_aperture -= DOOR_SPEED * 2;
		_aperture = clamp(_aperture, 0.0f, 1.0f);
	}
	//If we're not open, don't let the player through.
	if (knock_knock && Collide(player))
		Rebuff(_through * GLvector2(-1, -1));
	//If the player just got close, play open sound.
	if (knock_knock && closed && !_locked)
		AudioPlay("skate", _origin);
	//If the door just now slammed shut...
	if (!closed && _aperture == 0.0f) {
		AudioPlay("collide", _origin);
		ParticleDebris(_origin + _move_direction * -0.4f, 0.1f, 4, 2.0f);
	}
	//See if player just went through left-facing door.
	if (player_near && !_triggered && _facing == DOOR_LEFT && player.x > _origin.x)
		Trigger();
	//See if player just went through right-facing door.
	if (player_near && !_triggered && _facing == DOOR_RIGHT && player.x < _origin.x)
		Trigger();
	//See if player just went through down-facing door.
	if (player_near && !_triggered && _facing == DOOR_DOWN && player.y < _origin.y)
		Trigger();
	//See if player just went through up-facing door.
	if (player_near && !_triggered && _facing == DOOR_UP && player.y > _origin.y)
		Trigger();
	_line.start = _current_position + _move_direction * DOOR_HALF_HEIGHT;
	_line.end = _current_position - _move_direction * DOOR_HALF_HEIGHT;
}

/*-----------------------------------------------------------------------------
Pickup class - A floating object that can give the player stuff.
-----------------------------------------------------------------------------*/

#define PICKUP_BOB      0.01f

void fxPickup::InitGun(GLvector2 position, int id)
{
	const Projectile*   p_info;

	_type = PICKUP_GUN;
	_origin = position;
	_active = true;
	_projectile_id = id;
	p_info = EnvProjectileFromId(_projectile_id);
	_sprite = p_info->_icon;
	_sprite_color = p_info->_color;
	_sprite_position = _origin;
	_sprite_size = 0.5f;
	_sprite_angle = -15.0f + RandomFloat() * 30.0f;
	_bob_cycle = RandomFloat();
	_touching_player = TouchingPlayer();
	_can_grab = false; //You can't pick it up until we have one frame of NOT touching.
	AudioPlay("drop", _origin);
	ParticleSparks(_origin, _sprite_color, 10);
	ParticleBloom(_origin, GLrgba(1, 1, 1), _sprite_size, 500);
	ParticleBloom(_origin, _sprite_color, 10.0f, 500);
}

bool fxPickup::PlayerAlreadyHasThis()
{
	const Projectile*   p_info = EnvProjectileFromId(_projectile_id);
	int                 weapon;
	int                 player_has;

	//Figure out which slot this weapon belongs in.
	weapon = PLAYER_WEAPON_SECONDARY;
	if (p_info->_slot == PROJECTILE_PRIMARY)
		weapon = PLAYER_WEAPON_PRIMARY;
	player_has = Player()->Weapon(weapon)->Info()->_id;
	//If the player already has the same weapon, don't bother switching.
	if (player_has == _projectile_id)
		return true;
	return false;
}

void fxPickup::GiveGun()
{
	const Projectile*   p_info = EnvProjectileFromId(_projectile_id);
	int                 weapon;
	int                 old_id;

	//Figure out which slot this weapon belongs in.
	weapon = PLAYER_WEAPON_SECONDARY;
	if (p_info->_slot == PROJECTILE_PRIMARY)
		weapon = PLAYER_WEAPON_PRIMARY;
	old_id = Player()->Weapon(weapon)->Info()->_id;
	//If the player already has the same weapon, don't bother switching.
	if (old_id == _projectile_id)
		return;
	//Give them this new weapon.
	Player()->Weapon(weapon)->Equip(p_info);
	//Now drop the other weapon where they're standing.
	fxPickup*   p = new fxPickup;

	p->InitGun(PlayerPosition(), old_id);
	EntityFxAdd(p);

	fxMessage*      m;
	m = new fxMessage;
	m->Init(_origin, p_info->_title.c_str());
	EntityFxAdd(m);

	_active = false;
	if (weapon != PLAYER_WEAPON_SECONDARY)
		AudioPlay("missile_get");
	else
		AudioPlay("missile_load");
}

void fxPickup::Update()
{
	bool    touching_now;

	_bob_cycle += _bob_up ? PICKUP_BOB : -PICKUP_BOB;
	if (_bob_cycle < 0.0f)
		_bob_up = true;
	if (_bob_cycle > 1.0f)
		_bob_up = false;
	_bob = syMathScalarCurve(_bob_cycle);
	_bob = (_bob - 0.5f) * 2.0f;
	_sprite_position.y = _origin.y + _bob * 0.2f;
	touching_now = TouchingPlayer();
	//If the player just now bumped into us...
	if (touching_now && _can_grab && !PlayerAlreadyHasThis()) {
		string  offer;

		offer = StringSprintf("to take the %s.", EnvProjectileFromId(_projectile_id)->_title.c_str());

		PlayerOfferAccess(0.0f, offer.c_str());
		if (PlayerAccessActivated())
			GiveGun();
	}
	if (touching_now && _can_grab && PlayerAlreadyHasThis()) {
		HudMessage(StringSprintf("You already have the %s.", EnvProjectileFromId(_projectile_id)->_title.c_str()));
	}

	_touching_player = touching_now;
	if (!touching_now)
		_can_grab = true;
}

void fxPickup::Render()
{
	RenderQuad(_sprite_position, _sprite, _sprite_color, _sprite_size, _sprite_angle, DEPTH_FX, false);
	RenderQuad(_sprite_position, SPRITE_GLOW, _sprite_color*0.2f, _sprite_size, 0, DEPTH_FX - 0.1f, true);
}

bool fxPickup::TouchingPlayer()
{
	GLvector2   offset;

	offset = PlayerPosition() - _sprite_position;
	if (abs(offset.x) < _sprite_size && abs(offset.y) < _sprite_size)
		return true;
	return false;
}

/*-----------------------------------------------------------------------------
Dust class - Unlike other fx objects, this one should only be instanced ONCE.
It maintains a field of floating dust motes to give the game a certain visual
depth.
-----------------------------------------------------------------------------*/

#define DRIFT_SPEED         0.01f
#define SPIN_SPEED          8.0f
#define DUST_UPDATE         100
#define DUST_SIZE           0.13f

void fxDust::Init()
{
	float       min_depth;
	float       max_depth;
	float       depth_range;
	float       scale;

	max_depth = 1.5f;
	min_depth = -3.0f;// DEPTH_BG_NEAR;
	_field.x = EnvPlayerVisionRadius() * 3.0f;
	_field.y = _field.x / RenderAspect();
	_update = 0;
	depth_range = max_depth - min_depth;
	for (unsigned i = 0; i < MAX_DUST; i++) {
		_mote[i].position.x = RandomFloat() * _field.x * 2.0f;
		_mote[i].position.y = RandomFloat() * _field.y * 2.0f;
		scale = RandomFloat();
		//Place the mote within our range. Not too close to the camera, not too far into the screen.
		_mote[i].depth = scale * depth_range + min_depth;
		//Scale it so distant motes are twice the size of close ones, so we can still see them.
		_mote[i].size = (2.0f - scale * 1.0f) * DUST_SIZE;
		_mote[i].drift.x = (RandomFloat() - 0.5f) * DRIFT_SPEED;
		_mote[i].drift.y = (RandomFloat() - 0.125f) * DRIFT_SPEED;
		_mote[i].angle = RandomFloat() * 360.0f;
		_mote[i].spin = (RandomFloat() - 0.5f) * SPIN_SPEED;
		_mote[i].color = GLrgba(0.5f, 0.5f, 0.5f);
		_mote[i].sprite = SPRITE_MOTE;
	}
}

void fxDust::Update()
{
	Mote*     m;
	GLbbox    screen;
	bool			moved;

	screen.Clear();
	screen.ContainPoint(CameraPosition2D() - _field);
	screen.ContainPoint(CameraPosition2D() + _field);
	_field.x = EnvPlayerVisionRadius() * 3.0f;
	_field.y = _field.x / RenderAspect();
	_update = (_update + DUST_UPDATE) % MAX_DUST;
	for (unsigned i = 0; i < DUST_UPDATE; i++) {
		m = &_mote[(_update + i) % MAX_DUST];
		m->position += m->drift;
		moved = false;
		if (m->position.x < screen.pmin.x) {
			moved = true;
			m->position.x += _field.x * 2;
		}
		if (m->position.y < screen.pmin.y) {
			moved = true;
			m->position.y += _field.y * 2;
		}
		if (m->position.x > screen.pmax.x) {
			m->position.x -= _field.x * 2;
			moved = true;
		}
		if (m->position.y > screen.pmax.y) {
			m->position.y -= _field.y * 2;
			moved = true;
		}
		if (moved && WorldFinalBossKilled()) {
			m->color = GLrgbaUnique(GameTick());
			m->sprite = SPRITE_NOVA;
			m->size = (2.2f - RandomFloat()) * DUST_SIZE;
			m->drift.x = (RandomFloat() - 0.5f) * DRIFT_SPEED * 2;
			m->drift.y = (RandomFloat() + 0.25f) * DRIFT_SPEED * 2;
			m->spin = (RandomFloat() - 0.5f) * SPIN_SPEED * 2;
		}
		m->angle += m->spin;
	}
}

void fxDust::Render(GLrgba c)
{
	Mote*     m;

	if (!EnvValueb(ENV_RENDER_DUST))
		return;
	for (unsigned i = 0; i < MAX_DUST; i++) {
		m = &_mote[i];
		RenderQuad(m->position, m->sprite, c * m->color, m->size, m->angle, m->depth, true);
	}
}

/*-----------------------------------------------------------------------------
This creates a forcefield that is active ONLY if there is a boss fight going on.
It should be centered on the last room in the zone.
-----------------------------------------------------------------------------*/

#define FF_SHOVE    0.12f
#define FF_SIZE     (PAGE_HALF - 1)
#define FF_BOUND    2.0f
#define FF_BBOX     (FF_SIZE+FF_BOUND)

void fxForcefield::Init(GLvector2 pos, ForcefieldType t)
{
	//The given position is the upper-left of this room. Set our origin to the center.
	_origin = pos + GLvector2(PAGE_HALF, PAGE_HALF);
	_color = (WorldLampColor() + GLrgba(1, 1, 1)) / 2.0f;
	_active = true;
	_bbox.Clear();
	_bbox.ContainPoint(_origin - GLvector2(FF_BBOX, FF_BBOX));
	_bbox.ContainPoint(_origin + GLvector2(FF_BBOX, FF_BBOX));
	_sound_rebuff = "forcefield_bounce";
	_color_cycle = 0;
	_on = false;
	_type = t;

	//Place our four walls around that center.
	_position[0] = _origin + GLvector2(0, -FF_SIZE);//top
	_position[1] = _origin + GLvector2(FF_SIZE, 0);//right
	_position[2] = _origin + GLvector2(0, FF_SIZE);//bottom
	_position[3] = _origin + GLvector2(-FF_SIZE, 0);//left
	_size = GLvector2(FF_SIZE * 2, 1);
}

void fxForcefield::Update()
{
	GLvector2   offset;
	GLvector2   player;
	bool        nearby;
	float       distance;

	player = PlayerOrigin();

	int  last_room = WorldZone()->RoomCount() - 2;
	if (_type == FORCEFIELD_BOSS)
		_on = WorldBossGet() != NULL;
	else if (_type == FORCEFIELD_MOBS)
		_on = WorldZone()->RobotSpawnCount(last_room) != 0;
	else
		_on = false;
	if (!_bbox.Contains(player) || !_on) {
		AudioLoop(LOOP_FORCEFIELD, 1.0f, 0.0f);
		return;
	}
	offset = player - _origin;

	distance = max(abs(offset.x) - FF_SIZE, abs(offset.y) - FF_SIZE);
	distance = (FF_BOUND - clamp(distance, 0.0f, FF_BOUND)) / FF_BOUND;
	AudioLoop(LOOP_FORCEFIELD, 1.0f, distance);
	if (EnvValueb(ENV_NOCLIP))
		return;

	nearby = false;
	if (abs(offset.x) > abs(offset.y)) {
		//Right wall
		if (offset.x > 0.0f && offset.x < FF_SIZE) {
			player.x = max(player.x, _origin.x + FF_SIZE);
			PlayerShove(GLvector2(FF_SHOVE, 0));
			nearby = true;
		}
		//left wall
		if (offset.x < 0.0f && offset.x > -FF_SIZE) {
			player.x = min(player.x, _origin.x - FF_SIZE);
			PlayerShove(GLvector2(-FF_SHOVE, 0));
			nearby = true;
		}
	}
	else { //we're coming in from top or bottom
		//top wall
		if (offset.y < 0.0f && offset.y > -FF_SIZE) {
			player.y = min(player.y, _origin.y - FF_SIZE);
			PlayerShove(GLvector2(0, -FF_SHOVE));
			nearby = true;
		}
		//bottom wall
		if (offset.y > 0.0f && offset.y < FF_SIZE) {
			player.y = min(player.y, _origin.y + FF_SIZE);
			PlayerShove(GLvector2(0, FF_SHOVE));
			nearby = true;
		}
	}
	if (nearby) {
		GLvector2   dir = GLvector2(player - _origin).Normalized() * 0.1f;
		AudioPlay(_sound_rebuff, 1.0f);
		PlayerOriginSet(player);
		ParticleSparks(player, _color, 3);
		ParticleSparks(player, dir, _color, 3);
	}
}

void fxForcefield::Render()
{
	if (!_on)
		return;
	if (EnvValueb(ENV_BBOX)) {
		glColor3f(1, 1, 1);
		glDisable(GL_TEXTURE_2D);
		_bbox.Render();
		glEnable(GL_TEXTURE_2D);
	}
	RenderQuad(_position[0], SPRITE_BEAM, _color, _size, 0, 0, true);
	RenderQuad(_position[1], SPRITE_BEAM, _color, _size, 90, 0, true);
	RenderQuad(_position[2], SPRITE_BEAM, _color, _size, 180, 0, true);
	RenderQuad(_position[3], SPRITE_BEAM, _color, _size, 270, 0, true);
}

/*-----------------------------------------------------------------------------
This is the hat that the player wears on their fabulous head. If they take
any damage, it falls off.
-----------------------------------------------------------------------------*/

void fxHat::Init(SpriteEntry s, float size, GLrgba color)
{
	_id = fx_id++;
	_active = true;
	_sprite = s;
	_size = size;
	_origin = GLvector2();
	_movement = GLvector2();
	_spin = 0.0f;
	_color = color;
	_worn = false;
}

void fxHat::Render()
{
	RenderQuad(_origin, _sprite, _color, _size, _angle, 0, false);
}

void fxHat::Wear(GLvector2 position, float angle)
{
	_origin = position;
	_angle = angle;
	_worn = true;
}

void fxHat::Drop(GLvector2 movement, float spin)
{
	_spin = spin;
	_movement = movement;
	_worn = false;
}

void fxHat::Update()
{
	if (_worn)
		return;
	_origin += _movement;
	_movement.y += GRAVITY;
	_angle += _spin;
	if (Collision(_origin)) {
		_movement = GLvector2();
		_spin = 0;
	}
}