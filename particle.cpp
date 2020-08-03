/*-----------------------------------------------------------------------------

  Particle.cpp

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "bodyparts.h"
#include "camera.h"
#include "collision.h"
#include "env.h"
#include "game.h"
#include "noise.h"
#include "particle.h"
#include "player.h"
#include "random.h"
#include "render.h"
#include "system.h"

#define PARTICLE_LIMIT				2500
#define MAX_UPDATE						2000
#define DEBRIS_LIFESPAN				2000
#define PARTICLE_COUNT_BOOST	1			//ONLY FOR TESTING. Uselessly multiplies the number of particles.

static int      seed;

class Particle
{
	SpriteEntry _sprite;
	int         _time_begin;
	int         _time_end;
	int         _frame;
	bool        _dead;
	GLrgba      _color;
	GLrgba      _color_render;
	bool        _gravity;
	bool        _glow;
	bool        _fade;
	bool        _trail;
  bool        _collision;
	float       _size;
	float       _size_start;
	float       _size_end;
	GLvector2   _position;
	GLvector2   _velocity;
	int         _lifespan;
	float       _spin;
	float       _fangle;

public:
	bool        Dead() { return _dead; }
	void        Init(SpriteEntry sprite, GLrgba color, GLvector2 position, bool gravity, bool glow, int lifespan, float size, float scatter, float spin);
	void        Accelerate(GLvector2 accel) { _velocity += accel; }
  void        CollisionSet (bool enable) { _collision = enable; }
  void        FadeSet (bool enable) { _fade = enable; }
	void        TrailSet(bool enable) { _trail = enable; }
	void        ScaleSet(float start, float end) { _size_start = start; _size_end = end; }
	void        VelocitySet(GLvector2 v) { _velocity = v; }
	void        Update();
	void        Render();
};

static vector<Particle>     queue;
static vector<Particle>     particle;
static GLuvFrame*           smoke;
static GLuvFrame*           spark;
static GLuvFrame*           glow[2];
static GLuvFrame*           debris[2];
static GLuvFrame*           rubble[2];
static int                  next_cleanup;
static int                  live_particles;
static int									update_particle;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static bool overloaded ()
{
	if (particle.size () + queue.size () > PARTICLE_LIMIT)
		return true;
	return false;
}

/*-----------------------------------------------------------------------------
Module stuff
-----------------------------------------------------------------------------*/

void ParticleInit()
{
	spark = SpriteMapLookup(SPRITE_SPARK);
	glow[0] = SpriteMapLookup(SPRITE_GLOW);
	glow[1] = SpriteMapLookup(SPRITE_FOG);
	smoke = SpriteMapLookup(SPRITE_SMOKE);
	debris[0] = SpriteMapLookup(SPRITE_DEBRIS1);
	debris[1] = SpriteMapLookup(SPRITE_DEBRIS2);
	rubble[0] = SpriteMapLookup(SPRITE_RUBBLE1);
	rubble[1] = SpriteMapLookup(SPRITE_RUBBLE2);
}

void ParticleBloom(GLvector2 origin, GLrgba color, float size, int lifespan)
{
	Particle    p;

	if (overloaded ())
		return;
	p.Init (SpriteEntryLookup ("Circle"), color, origin, false, true, lifespan, size, size / 100.0f, 50.0f);
	p.ScaleSet (0, size);
	p.FadeSet(true);
	queue.push_back(p);
}

void ParticleSmoke(GLvector2 origin, float size, int count)
{
	Particle    p;

	if (overloaded ())
		return;
	count *= PARTICLE_COUNT_BOOST;
	for (int i = 0; i < count; i++) {
		p.Init(SPRITE_SMOKE, GLrgba(), origin, false, false, 1500 + RandomVal() % 1000, size, 0.01f, 1);
		p.Accelerate(GLvector2(0, -0.01f));
		p.FadeSet(true);
		queue.push_back(p);
	}
}

void ParticleExplode(GLvector2 origin, GLrgba color, int count, float size)
{
	Particle    p;
	SpriteEntry sprite;

	if (overloaded ())
		return;
	count *= PARTICLE_COUNT_BOOST;
	for (int i = 0; i < count; i++) {
		if (i % 2)
			sprite = SPRITE_DEBRIS1;
		else
			sprite = SPRITE_DEBRIS2;
		p.Init(sprite, color, origin, true, false, 1200, size, 0.06f, 23);
		p.FadeSet(true);
		p.TrailSet(true);
		queue.push_back(p);
	}
}

void ParticleSprite(GLvector2 origin, GLvector2 movement, GLrgba color, SpriteEntry sprite, int count, float size, bool glow)
{
	Particle    p;

	if (overloaded ())
		return;
	count *= PARTICLE_COUNT_BOOST;
	for (int i = 0; i < count; i++) {
		p.Init(sprite, color, origin, false, glow, 1200, size, 0, 0);
		p.Accelerate(movement);
		p.FadeSet(true);
		queue.push_back(p);
	}
}

void ParticleGlow(GLvector2 origin, GLvector2 movement, GLrgba color1, GLrgba color2, int count, float size)
{
	Particle    p;

	if (overloaded ())
		return;
	count *= PARTICLE_COUNT_BOOST;
	for (int i = 0; i < count; i++) {
		p.Init(SPRITE_GLOW, color1, origin, false, true, 250 + RandomVal() % 1500, size, 0.01f, 3);
		p.Accelerate(movement);
		p.FadeSet(true);
		queue.push_back(p);
		seed--;
		p.Init(SPRITE_SPARK, color2, origin, false, true, 250 + RandomVal() % 1500, size, 0.01f, 14);
		p.Accelerate(movement);
		p.FadeSet(true);
		queue.push_back(p);
	}
}

void ParticleSparks(GLvector2 origin, GLvector2 movement, GLrgba color, int count)
{
	Particle    p;

	if (overloaded ())
		return;
	count *= PARTICLE_COUNT_BOOST;
	for (int i = 0; i < count; i++) {
		p.Init(SPRITE_SPARK, color, origin, false, true, 250 + RandomVal() % 1500, 0.07f+RandomFloat()*0.15f, 0.05f, 10);
		p.Accelerate(movement);
		p.FadeSet(true);
		queue.push_back(p);
	}
}

void ParticleSparks(GLvector2 origin, GLrgba color, int count)
{
	Particle    p;

	if (overloaded ())
		return;
	count *= PARTICLE_COUNT_BOOST;
	for (int i = 0; i < count; i++) {
		p.Init(SPRITE_SPARK, color, origin, false, true, 1000 + RandomVal() % 1000, 0.16f, 0.01f, 10);
		queue.push_back(p);
	}
}

void ParticleDebris(GLvector2 origin, float size, int count, float speed)
{
	Particle    p;
	SpriteEntry sprite;

	if (overloaded ())
		return;
	count *= PARTICLE_COUNT_BOOST;
	for (int i = 0; i < count; i++) {
		if (i % 2)
			sprite = SPRITE_DEBRIS1;
		else
			sprite = SPRITE_DEBRIS2;
    p.Init (sprite, GLrgba (), origin, true, false, DEBRIS_LIFESPAN + RandomVal () % DEBRIS_LIFESPAN, size, 0.05f*speed, 3);
    p.CollisionSet (true);
		queue.push_back(p);
	}
}

void ParticleDebris (GLvector2 origin, float size, int count, float speed, GLvector2 direction)
{
  Particle    p;
  SpriteEntry sprite;

	if (overloaded ())
		return;
	count *= PARTICLE_COUNT_BOOST;
	for (int i = 0; i < count; i++) {
    if (i % 2)
      sprite = SPRITE_DEBRIS1;
    else
      sprite = SPRITE_DEBRIS2;
    p.Init (sprite, GLrgba (), origin, true, false, DEBRIS_LIFESPAN + RandomVal () % DEBRIS_LIFESPAN, (RandomFloat ()+0.33f) * size, 0.05f*speed, 3);
    p.VelocitySet (direction+GLvector2 (RandomFloat () - 0.5f, RandomFloat () - 0.5f)*speed);
    p.CollisionSet (true);
    queue.push_back (p);
  }
}

void ParticleBlood (GLvector2 origin, GLrgba color, float size, int count, float speed, GLvector2 direction)
{
	Particle    p;
	SpriteEntry sprite;
	GLrgba			color_blood;
	GLrgba			color_dark;
	GLrgba			color_light;

	if (overloaded ())
		return;
	color_dark = color * 0.33f;
	color_light = color;
	count *= PARTICLE_COUNT_BOOST;
	size = clamp (size, 0.05f, 0.1f);
	sprite = SpriteEntryLookup ("Circle");
	for (int i = 0; i < count; i++) {
		color_blood = Lerp (color_dark, color_light, RandomFloat ());
		p.Init (sprite, color_blood, origin, true, true, DEBRIS_LIFESPAN + RandomVal () % DEBRIS_LIFESPAN, (RandomFloat () + 0.33f) * size, 0.01f, (RandomFloat () - 0.5f) * 100.0f);
		p.VelocitySet (direction * speed + (RandomFloat () - 0.5f)*speed);
		p.CollisionSet (true);
		queue.push_back (p);
	}
}


void ParticleRubble(GLvector2 origin, float size, int count)
{
	Particle    p;
	SpriteEntry sprite;

	if (overloaded ())
		return;
	count *= PARTICLE_COUNT_BOOST;
	for (int i = 0; i < count; i++) {
		if (i % 2)
			sprite = SPRITE_RUBBLE1;
		else
			sprite = SPRITE_RUBBLE2;
		p.Init(sprite, GLrgba(), origin, true, false, 1000, size, 1.1f*size, 5);
		queue.push_back(p);
	}
}

void ParticleUpdate()
{
	if (GamePaused())
		return;
	//We need to do a run of the list and pull out any members that have died.
	if (GameTick() > next_cleanup) {
		for (unsigned i = 0; i < particle.size(); i++) {
			if (particle[i].Dead()) {
				unsigned   last = particle.size() - 1;
				if (i < last) //If we're not at the end of the list.
					particle[i] = particle[last];
				particle.pop_back();
			}
		}
		next_cleanup = GameTick() + 1000;
	}

	//Since particles can spawn other particles, we don't want to add them
	//in the middle of an update. So we stick new particles in a queue.
	//Here we take the queue from the previous frame and add them.
	if (!queue.empty() && live_particles < PARTICLE_LIMIT) {
		particle.insert(particle.end(), queue.begin(), queue.end());
	}
	queue.clear();
	//Update the particles, and we're done.
	if (particle.empty ())
		return;
	if (update_particle >= particle.size ())
		update_particle = 0;
	
	int			update_count = 0;
	int			first_particle = update_particle;

	live_particles = 0;
	while (update_count < MAX_UPDATE) {
		if (!particle[update_particle].Dead ()) {
			live_particles++;
			particle[update_particle].Update ();
		}
		update_count++;
		update_particle = (update_particle + 1) % particle.size ();
		if (update_particle == first_particle)
			break;
	}
}

unsigned ParticleCount()
{
	return particle.size();
}

void ParticleRender()
{
  if (!EnvValueb (ENV_RENDER_PARTICLES))
    return;
	for (unsigned i = 0; i < particle.size(); i++)
		particle[i].Render();
	RenderQuads();
}

/*-----------------------------------------------------------------------------
The particle class members
-----------------------------------------------------------------------------*/
void Particle::Init(SpriteEntry sprite, GLrgba color, GLvector2 position, bool gravity, bool glow, int lifespan, float size, float scatter, float spin)
{
	_color = _color_render = color;
	_position = position;
	_gravity = gravity;
	_glow = glow;
	_size_start = _size_end = _size = size;
	_fade = false;
	_trail = false;
  _collision = false;
	_frame = 0;
	_sprite = sprite;
	_velocity = GLvector2(Noisef(seed) * scatter, Noisef(seed + 1) * scatter) - (scatter / 2);
	seed += 2;
	//The given spin rate is the max spin speed in either direction
	_spin = (Noisef(seed++) * spin * 2) - spin;
	_fangle = Noisef(seed++) * 360.0f * spin;
	_lifespan = lifespan;
	_time_begin = GameTick();
	_time_end = _time_begin + _lifespan;
	_dead = false;
}

void Particle::Render()
{
	GLquad    q;

	q = SpriteMapQuad((int)_fangle);
	if (_dead)
		return;
	RenderQuad(_position, _sprite, _color_render, _size, _fangle, _glow ? DEPTH_FX_GLOW : DEPTH_FX, _glow);
}

void Particle::Update()
{
	int       now;
	GLvector2 distance;
  GLvector2 old_position;
	GLvector  camera;
	float     max_distance;
	float     delta;

	_frame++;
	now = GameTick();
	camera = CameraPosition();
	delta = (float)(now - _time_begin) / (float)_lifespan;
	distance = _position - GLvector2(camera.x, camera.y);
	max_distance = camera.z * 2.5f;
	if (abs(distance.x) > max_distance || abs(distance.y) > max_distance)
		_dead = true;
	if (now > _time_end)
		_dead = true;
	if (_dead)
		return;

	if (_gravity)
		_velocity.y += GRAVITY;
	if (_fade) {
		float diminish;
		diminish = 1.0f - delta;
		if (_glow)
			_color_render = _color * diminish;
		else
			_color_render.alpha = diminish / 3.0f;
	}
	if (_trail && (_frame % 8) == 0)
		ParticleSparks(_position, GLvector2(), _color, 1);
	_size = Lerp(_size_start, _size_end, delta);
	_fangle += _spin;
  old_position = _position;
	_position += _velocity;
  //Collision only applies to walls.
  if (_collision) {
    GLvector2   wall;
    if (Collision (_position, &wall, NULL)) {
      _velocity = GLreflect2 (_velocity, wall);
      _position = old_position + _velocity;
      //lose vertical momentum when colliding, or else they act like
      //bouncy balls.
      _velocity.y *= 0.5f;
    }
  }
	_velocity *= 0.97f;
}