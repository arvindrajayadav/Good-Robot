#ifndef SPRITE_H
#define SPRITE_H

enum SpriteEntry;

class SpriteUnit
{
	GLvector2     _position;
	GLvector2     _eye_offset;
	GLcoord2      _sprite_override;
	GLquad        _quad;
	GLrgba        _color;
	float         _angle;
	float         _size;
	GLvector2     _look;
	class bodyEye _eye;
	SpriteEntry   _sprite_entry;
	int						_tick_stop_shaking;
	bool          _use_eye;
	bool          _enabled;

public:
	void          Init();
  void          Init (SpriteEntry index, float body_size, GLrgba body_color, SpriteEntry eye, SpriteEntry iris, GLrgba eye_color, float eye_size, GLvector2 eyepos);
  void          Init (GLcoord2 sprite, float body_size, GLrgba body_color, SpriteEntry eye, SpriteEntry iris, GLrgba eye_color, float eye_size, GLvector2 eyepos);
	void          Init(SpriteEntry index, float body_size, GLrgba body_color);
	void          Init(GLcoord2 sprite, float body_size, GLrgba body_color);
	float         Angle() { return _angle; }
	bool          Collide(GLvector2 pos);
	void          Render();
	void          RenderEye ();
	void          RenderIris ();
  void          EntrySet (SpriteEntry e) { _sprite_entry = e; }
	void          EyeEnable(bool enable) { _use_eye = enable; }
	void          EyeMove(GLvector2 eyepos) { _eye_offset = eyepos; }
	void          Move(GLvector2 position, float angle = 0);
	void          Look(GLvector2 direction);
	void          Look(float direction);
	SpriteEntry		Sprite () { return _sprite_entry;  }
	void          SetColor(GLrgba c);
	void					Shake (int tick_stop);
	GLvector2     Position() { return _position; }
	float         Size() { return _size; }
	void          Blink(float amount);
};

class SpriteBox
{
	GLquad      _quad;
	GLuvFrame   _uv;
	GLrgba      _color;
	bool        _use_color;

public:
	void        Init(GLvector2 pos, float size, float angle, int srite_group, int sprite_index);
	void        Render();
	void        ColorSet(GLrgba c);
};

#endif // SPRITE_H