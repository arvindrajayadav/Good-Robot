#ifndef AVATAR_H
#define AVATAR_H

#include "bodyparts.h"
#include "character.h"
#include "env.h"
#include "fx.h"
#include "player.h"

struct BodyPart
{
	class SpriteUnit      sprite;
	GLvector2             momentum;
	int                   angle;
	int                   spin;
	bool                  active;
};

enum RespawnStep
{
	RESPAWN_BEGIN,
	RESPAWN_LAUNCHER,
	RESPAWN_PRIMARY,
	RESPAWN_HEAD,
	RESPAWN_POWERUP,
	RESPAWN_DONE
};

class Avatar
{
	enum {
		HEAD,
		TORSO,
		PRIMARY,
		SECONDARY,
		CORE,
		PARTS
	};

	GLvector2   _origin;
	int         _character;
	GLrgba      _self_color;
	GLvector2   _last_momentum;
	GLvector2   _center;
	GLbbox2     _bbox;
	int         _respawn_begin;
	int         _respawn_step;
	BodyPart    _body[PARTS];
	bodyTail    _tail;
  SpriteEntry _eye;
  SpriteEntry _iris;
	bool        _dead;
	bool        _respawning;
	float       _blink;
	float       _size;

  void        DoHand (GLvector2 pos, int part, int weapon, PlayerStats* ps, float aim);
	void        DoExplode();
	void        DoRespawn();
	void        DoParts (const PlayerStats* stats);
	void        DoMove(GLvector2 momentum, GLvector2 look, float aim_angle, PlayerStats* stats);

public:
	void        Blink(float amount) { _blink = amount; }
	GLvector2   Center() { return _center; }
	bool        Collide(GLvector2 pos);
	void        ColorSet(GLrgba c) { _self_color = c; }
	bool        Dead() { return _dead; }
	void        Init(int character);
	void        Kill();
	void        Move(GLvector2 position, GLvector2 momentum, GLvector2 look, float aim_angle, PlayerStats* stats);
	GLvector2		Momentum () { return _last_momentum;  }
	void        Render();
	void        Spawn(GLvector2 position, PlayerStats stats, bool instant);
	float       Size() { return _body[TORSO].sprite.Size(); }
	void        Scale(float size) { _size = size; }
	GLvector2   OriginLaser() { return _body[PRIMARY].sprite.Position(); }
	GLvector2   OriginLauncher() { return _body[SECONDARY].sprite.Position(); }
	GLvector2   OriginHead() { return _body[HEAD].sprite.Position(); }
};

#endif // AVATAR_H
