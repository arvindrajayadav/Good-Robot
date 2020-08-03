/*-----------------------------------------------------------------------------

 Avatar.cpp

 Animates the sprites that make up the player character.

 Good Robot
 (c) 2013 Shamus Young

 -----------------------------------------------------------------------------*/

#include "master.h"
#include "audio.h"
#include "avatar.h"
#include "particle.h"
#include "random.h"
#include "render.h"
#include "sprite.h"

#define EXPLODE_SPEED     0.02f
#define BLINK_LENGTH      160
#define BLINK_INTERVAL    6000
#define RESPAWN_SPEED     250

//This mess is to arrange all the parts of our animated avatar
#define DEFAULT_SIZE      0.1f
#define BODY_SIZE         (_size)
#define TORSO_SIZE        (1.2f * BODY_SIZE)
#define HEAD_SIZE         (1.1f * BODY_SIZE)
//#define EYE_SIZE          (4.5f * HEAD_SIZE)
#define EYE_SIZE          (0.5f)
#define HEAD_POS          (GLvector2 (0, -2.0f) * BODY_SIZE)
#define TAIL_SIZE         (BODY_SIZE * 0.8f)
#define LAUNCHER_SIZE     (0.9f * BODY_SIZE)
#define LAUNCHER_POS      (GLvector2 (-0.0f, -0.4f) * BODY_SIZE)
#define LAUNCHER_DIST     (1.3f * BODY_SIZE)
#define PRIMARY_SIZE        (0.9f * BODY_SIZE)
#define PRIMARY_POS         (GLvector2 (0.4f, -0.45f) * BODY_SIZE)
#define PRIMARY_DIST        (-1.7f * BODY_SIZE)
#define BREATHE_INTERVAL  3000
#define BREATHE_MOVEMENT  (GLvector2 (0, 0.2f) * BODY_SIZE)
#define SHIELD_SIZE       (5.5f * BODY_SIZE)
#define SHIELD_POS        (GLvector2 (0, -0.3f) * BODY_SIZE)

/*-----------------------------------------------------------------------------

 -----------------------------------------------------------------------------*/

void Avatar::Init(int character_index)
{
	Character   c = EnvCharacter(character_index);

	_eye = (SpriteEntry)(c._eye_number + SPRITE_EYE0);
	_iris = (SpriteEntry)(c._eye_number + SPRITE_IRIS0);
	_self_color = GLrgba(1, 1, 1);
	_character = character_index;
	_last_momentum = GLvector2();
	_blink = 0;
	_dead = true;
	_size = DEFAULT_SIZE;
	_tail.Init(GLvector2(), GLrgba(1, 1, 1), TAIL_SIZE);
	_body[TORSO].sprite.Init(c._sprite_torso, c._size_torso, GLrgba());
	_body[CORE].sprite.Init(SPRITE_CORE, c._size_torso, GLrgba());
	_body[HEAD].sprite.Init(c._sprite_head, c._size_head, GLrgba(), _eye, _iris, GLrgba(0, 1, 1), c._size_eye, GLvector2(0, 1.9f));
	_body[SECONDARY].sprite.Init(c._sprite_launcher, c._size_launcher, GLrgba());
	_body[PRIMARY].sprite.Init(c._sprite_laser, c._size_laser, GLrgba());
}

void Avatar::DoParts(const PlayerStats* stats)
{
	Character     c = EnvCharacter(_character);
	int           launcher;

	launcher = SPRITE_LAUNCHER6;
	_body[TORSO].sprite.Init(c._sprite_torso, c._size_torso, GLrgba());
	_body[CORE].sprite.Init(SPRITE_CORE, c._size_torso, GLrgba());
	_body[HEAD].sprite.Init(c._sprite_head, c._size_head, GLrgba(), _eye, _iris, GLrgba(0, 1, 1), c._size_eye, GLvector2(0, 1.9f));
}

void Avatar::Spawn(GLvector2 position, PlayerStats stats, bool instant)
{
	_origin = position;
	_tail.Init(position, GLrgba(1, 1, 1), TAIL_SIZE);
	DoParts(&stats);
	if (instant) {
		_body[HEAD].sprite.EyeEnable(true);
		_dead = false;
		_respawning = false;
		_respawn_begin = GameTick() - RESPAWN_SPEED;
		_respawn_step = RESPAWN_HEAD;
		return;
	}
	_respawn_begin = GameTick();
	_respawn_step = RESPAWN_BEGIN;
	_respawning = true;
	_body[HEAD].sprite.EyeEnable(false);
}

void Avatar::Kill()
{
	if (_dead)
		return;
	_dead = true;
	for (int i = 0; i < PARTS; i++) {
		_body[i].active = true;
		_body[i].angle = 0;
		_body[i].momentum = _last_momentum;
		_body[i].momentum += GLvector2(RandomFloat() - 0.5f, -RandomFloat()) * EXPLODE_SPEED;
		_body[i].spin = RandomVal(10) - 5;
	}
	_body[HEAD].momentum.y -= RandomFloat() * EXPLODE_SPEED;
	_body[PRIMARY].momentum.x -= RandomFloat() * EXPLODE_SPEED;
	_body[SECONDARY].momentum.x -= RandomFloat() * EXPLODE_SPEED;
}

void Avatar::Render()
{
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(false);
	_tail.Render();
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < PARTS; i++)
		_body[i].sprite.Render();
	RenderQuads ();
	_body[HEAD].sprite.RenderEye ();
	RenderQuads ();
	glDepthFunc (GL_EQUAL);
	_body[HEAD].sprite.RenderIris ();
	RenderQuads ();
	glDepthFunc (GL_LEQUAL);

}

bool Avatar::Collide(GLvector2 pos)
{
	for (int i = 0; i < PARTS; i++)
		if (_body[i].sprite.Collide(pos))
			return true;
	return false;
}

void Avatar::DoExplode()
{
	for (int i = 0; i < PARTS; i++) {
		_body[i].sprite.EyeEnable(false);
		_body[i].sprite.Move(_body[i].sprite.Position() + _body[i].momentum, (float)_body[i].angle);
		_body[i].angle += _body[i].spin;
		_body[i].momentum.y += 0.0005f;
		_body[i].sprite.SetColor(GLrgba(0, 0, 0));
	}
}

void Avatar::DoRespawn()
{
	int       elapsed;
	float     animate;
	GLvector2 part_off[PARTS];

	elapsed = GameTick() - _respawn_begin;
	if (elapsed > RESPAWN_SPEED) {
		elapsed -= RESPAWN_SPEED;
		_respawn_step++;
		_respawn_begin = GameTick();
		if (_respawn_step == RESPAWN_DONE) {
			AudioPlay("skillup", _origin);
			ParticleGlow(_origin, GLvector2(), _self_color / 3, _self_color / 3, 4, BODY_SIZE);
			_body[HEAD].sprite.EyeEnable(true);
			_dead = false;
			_respawning = false;
		}
		else {
			AudioPlay("missile_get", _origin);
			ParticleDebris(_origin, BODY_SIZE / 5, 3);
		}
	}
	animate = ((float)elapsed / RESPAWN_SPEED);
	animate *= animate * animate;
	animate = 1.0f - animate;
	part_off[TORSO] = GLvector2(0, 3);
	part_off[PRIMARY] = GLvector2(-3, 0);
	part_off[SECONDARY] = GLvector2(3, 0);
	part_off[HEAD] = GLvector2(0, -3);
	if (_respawn_step == RESPAWN_BEGIN)
		part_off[TORSO] *= animate;
	else if (_respawn_step == RESPAWN_LAUNCHER) {
		part_off[TORSO] *= 0;
		part_off[SECONDARY] *= animate;
	}
	else if (_respawn_step == RESPAWN_PRIMARY) {
		part_off[TORSO] *= 0;
		part_off[SECONDARY] *= 0;
		part_off[PRIMARY] *= animate;
	}
	else if (_respawn_step == RESPAWN_HEAD) {
		part_off[TORSO] *= 0;
		part_off[SECONDARY] *= 0;
		part_off[PRIMARY] *= 0;
		part_off[HEAD] *= animate;
	}
	else if (_respawn_step == RESPAWN_POWERUP) {
		part_off[TORSO] *= 0;
		part_off[SECONDARY] *= 0;
		part_off[PRIMARY] *= 0;
		part_off[HEAD] *= 0;
	}

	GLvector2 shoulders;
	GLvector2 launcher_offset;
	GLvector2 laser_offset;
	GLvector2 head_offset;

	shoulders = GLvectorFromAngle(270);
	launcher_offset = shoulders * LAUNCHER_DIST;
	laser_offset = shoulders * PRIMARY_DIST;

	_body[TORSO].sprite.Move(_origin + part_off[TORSO]);
	_body[CORE].sprite.Move(_origin);
	_body[SECONDARY].sprite.Move(_origin + LAUNCHER_POS + launcher_offset + part_off[SECONDARY]);
	_body[HEAD].sprite.Move(_origin + HEAD_POS + part_off[HEAD]);
	_body[PRIMARY].sprite.Move(_origin + PRIMARY_POS + laser_offset + part_off[PRIMARY]);
}

void Avatar::DoHand(GLvector2 pos, int part, int slot, PlayerStats* ps, float aim)
{
	const Projectile*     p_info;
	PlayerWeapon*         w;
	float                 angle;

	w = ps->Weapon(slot);
	p_info = w->Info();
	_body[part].sprite.EntrySet(p_info->_hand);
	_body[part].sprite.SetColor(w->Color());
	if (p_info->_hand_animate == HAND_FIXED)
		angle = 0;
	else if (p_info->_hand_animate == HAND_AIM)
		angle = aim;
	else { // spin
		angle = w->Angle();
	}
	_body[part].sprite.Move(pos, angle);
}

void Avatar::DoMove(GLvector2 momentum, GLvector2 look, float aim_angle, PlayerStats* stats)
{
	float     lean;
	float     cycle;
	GLrgba    self_bright;
	GLvector2 launcher_offset;
	GLvector2 laser_offset;
	GLvector2 head_offset;
	GLvector2 breathe_delta;
	GLvector2 head_turn;
	GLvector2 shoulders;
	GLvector2 body_shift;

	DoParts(stats);
	if (_blink == 0) {
		int   blink;

		blink = GameTick() % BLINK_INTERVAL;
		if (blink < BLINK_LENGTH) {
			_body[HEAD].sprite.Blink(1 - (float)blink / BLINK_LENGTH);
		}
		else
			_body[HEAD].sprite.Blink(0);
	}
	else  //Put eyelids where specified.
		_body[HEAD].sprite.Blink(_blink);
	look.Normalize();
	head_turn.x = clamp(momentum.x, -0.02f, 0.02f);
	head_turn.y = 0.02f + clamp(momentum.y, -0.01f, 0.01f);
	//head_lift = clamp (momentum.y, -0.001f, 0.001f);
	_body[HEAD].sprite.Look(look * 0.5f);
	_body[HEAD].sprite.EyeMove(head_turn);
	//This sets up a sawtooth wave between 0 and 0.5f
	//This is used to move the_body up and down so it's not rigid at rest.
	cycle = (float)(GameTick() % BREATHE_INTERVAL) / BREATHE_INTERVAL;
	if (cycle > 0.5f)
		cycle = 1 - cycle;
	breathe_delta = BREATHE_MOVEMENT * cycle;
	//Color everything according to status. Laser shows energy, LAUNCHER goes black if
	//out of rockets, chest fades with shields.
	//self_color = EnvBulletColor (stats.Level (UPGRADE_SHOT_POWER));
	self_bright = (_self_color + GLrgba(1, 1, 1)) / 2;

	Character c = EnvCharacter(_character);

	_body[TORSO].sprite.SetColor(c._color);
	_body[CORE].sprite.SetColor(c._color * stats->ShieldsPercent());
	_body[HEAD].sprite.SetColor(c._color);
	_tail.ColorSet(c._color);
	//We use our momentum to determine how much to lean right or left.
	lean = momentum.x * 300;
	//This moves all the little sprites relative to each other.
	//It's unavoidably messy and ugly.
	body_shift = (momentum / 2);
	body_shift.x = clamp(body_shift.x, -0.01f, 0.01f);
	body_shift.y = clamp(body_shift.y, -0.01f, 0.01f);
	_body[TORSO].sprite.Move(_origin - body_shift, lean);
	_body[CORE].sprite.Move(_origin - body_shift, -lean);
	shoulders = GLvectorFromAngle(270 + lean);
	launcher_offset = shoulders * LAUNCHER_DIST;
	laser_offset = shoulders * PRIMARY_DIST;
	//head_offset = SpriteMapVectorRotate (HEAD_POS, 180+(int)lean);
	head_offset = SpriteMapVectorRotate(c._pos_head, 180 + (int)lean);
	//_body[SECONDARY].sprite.Move(_origin + c._pos_launcher + launcher_offset - body_shift - breathe_delta, aim_angle);
	_body[HEAD].sprite.Move(_origin + head_offset + breathe_delta, 0);
	//_body[PRIMARY].sprite.Move(_origin + c._pos_laser + laser_offset - body_shift - breathe_delta, aim_angle);

	GLvector2   pri_pos = _origin + c._pos_laser + laser_offset - body_shift - breathe_delta;
	GLvector2   sec_pos = _origin + c._pos_launcher + launcher_offset - body_shift - breathe_delta;

	DoHand(pri_pos, PRIMARY, PLAYER_WEAPON_PRIMARY, stats, aim_angle);
	DoHand(sec_pos, SECONDARY, PLAYER_WEAPON_SECONDARY, stats, aim_angle);
}

void Avatar::Move(GLvector2 position, GLvector2 momentum, GLvector2 look, float aim_angle, PlayerStats* stats)
{
	_last_momentum = momentum;
	//If we're dead, animate all the flying_body parts.
	_tail.Update(position, GameTick());
	_origin = position;
	_self_color = stats->Color();
	if (_respawning) {
		DoRespawn();
	}
	else if (_dead) {
		DoExplode();
	}
	else
		DoMove(momentum, look, aim_angle, stats);
	//If no special eyelid position is demanded, just blink at intervals.
	_bbox.Clear();
	for (int i = 0; i < PARTS; i++) {
		float     size = _body[i].sprite.Size();
		_bbox.ContainPoint(_body[i].sprite.Position() - GLvector2(-size, -size));
		_bbox.ContainPoint(_body[i].sprite.Position() - GLvector2(size, size));
	}
	_center = _bbox.Center();
}