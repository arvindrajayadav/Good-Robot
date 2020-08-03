#ifndef BODYPARTS_H
#define BODYPARTS_H

enum SpriteEntry;

class bodyEye
{
	GLvector2     _position;
	GLvector2     _look;
	GLuvFrame*    _uvmask;
	GLuvFrame*    _uvlight;
	GLrgba        _color_outer;
	GLrgba        _color_inner;
	float         _size;
	float         _blink;
	float         _angle;
	SpriteEntry   _sprite_eye;
	SpriteEntry   _sprite_iris;

public:
	void          Init(GLrgba color_out, GLrgba col_in, SpriteEntry eye, SpriteEntry iris, float size);
	void          Move(GLvector2 pos, GLvector2 look, float angle = 0);
	void          Render();
	void          RenderEye ();
	void          RenderIris ();
	void          SetColor(GLrgba c);
	void          Blink(float val) { _blink = val; }
};

#include "sprite.h"

class bodyLeg
{
	GLvector2   _origin;
	GLvector2   _foot;
	GLvector2   _tip;
	GLvector2   _knee_center;
	GLvector2   _knee_offset;   //Offset from the body to the knee. Used by clients.
	GLquad      _knee;
	SpriteUnit  _sprite_knee;   //A sprite used to cover up the ugly joint
	GLrgba      _color;
	GLvector 		_triangles[9];  //The triangles that form the black part of the leg
	float       _stride;        //The distance from knee to body when legs are extended.
	float       _current_stride;//The current distance from knee to body
	float       _current_floor; //The current floor elevation under the knee.
	float       _current_walk;  //
	float       _knee_size;
	float       _step_radius;   //The leg tip orbits the current floor at this distance.
	float       _angle;         //Where this leg is in the step animation.
	float       _knee_height;   //How far the knee is off the ground.
	bool        _foot_up;       //True if the foot isn't touching the ground.
	bool        _ready;
	bool        _extended;      //True if the leg should be stretching out to the full spread.
	std::string _sound_step;    //The id of the sound to play for footsteps.

public:
	GLvector2   Bumper()  { return GLvector2(_stride, 0); }
	void        Extend() { _extended = true; }
	void        Init(GLrgba c, float stride, float knee_height, float knee_size, float step_radius, float start_angle);
	void        ColorSet(GLrgba c) { _color = c; _sprite_knee.SetColor(c); }
	void        SoundStepSet(const std::string &id) { _sound_step = id; }
	void        Move(GLvector2 body);
	void        Render();
	GLvector2   Knee() { return _knee_offset; }
	void        Update();
};

struct fxTp  //tail point. Used by bodyTail
{
	GLvector2     left;
	GLvector2     right;
	int           created;
};

class bodyTail
{
	GLvector2     _origin;
	GLrgba        _color;
	GLrgba        _color_glow;
	float         _size;
	vector<fxTp>  _points;
public:
	void          Init(GLvector2 start, GLrgba color, float size);
	void          Update(GLvector2 position, int timestamp);
	void          Update(int timestamp);
	void          Render();
	bool          Active() { return _points.size() > 0; }
	void          ColorSet(GLrgba c) { _color = c; }
};

#endif // BODYPARTS_H