/*-----------------------------------------------------------------------------

  Bodypartscpp

  (c) 2013 Shamus Young

  This class handles the logic behind the cartoon eyes and legs used by
  sprites.

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "bodyparts.h"
#include "collision.h"
#include "particle.h"
#include "render.h"
#include "sprite.h"
#include "world.h"

#define WALK_SPEED            120
#define STEP_UP_INTERPOLATE   0.1f
#define TAIL_SPACING          0.15f
#define TAIL_LIFESPAN         400

/*-----------------------------------------------------------------------------
Tail class - glowing trail behind stuff
-----------------------------------------------------------------------------*/

void bodyTail::Init(GLvector2 start, GLrgba color, float size)
{
	_origin = start;
	_color = color;
	_color_glow = (_color + GLrgba(1, 1, 1)) / 2;
	_size = size;
	_points.clear();
}

void bodyTail::Update(int timestamp)
{
	while (_points.size() && (timestamp - _points[0].created) > TAIL_LIFESPAN)
		_points.erase(_points.begin());
}

void bodyTail::Update(GLvector2 position, int timestamp)
{
	GLvector2     delta;
	fxTp          p;
	float         angle;

	while (_points.size() && (timestamp - _points[0].created) > TAIL_LIFESPAN)
		_points.erase(_points.begin());
	delta = position - _origin;
	//Make sure origin and new pos are spaced apart enough to justify making a line.
	if (delta.Length() < TAIL_SPACING)
		return;
	//Special case when we're getting the chain started.
	if (_points.size() == 0) {
		angle = delta.Angle();
		p.right = GLvectorFromAngle(angle + 90) * _size;
		p.left + p.right * -1;
		p.left += _origin;
		p.right += _origin;
		p.created = timestamp;
		_points.push_back(p);
		p.right = GLvectorFromAngle(angle + 90) * _size;
		p.left = p.right * -1;
		p.left += position;
		p.right += position;
		p.created = timestamp;
		_points.push_back(p);
		_origin = position;
		return;
	}
	angle = delta.Angle();
	p.right = GLvectorFromAngle(angle + 90) * _size;
	p.left = p.right * -1;
	p.left += position;
	p.right += position;
	p.created = timestamp;
	_points.push_back(p);
	_origin = position;
}

void bodyTail::Render()
{
	GLuvFrame*  uv;
	float       x1, x2, y1, y2, yi;

	if (_points.size() < 2)
		return;
	glBlendFunc(GL_ONE, GL_ONE);
	glColor3fv(&_color.red);

	uv = SpriteMapLookup(SPRITE_TAIL);
	x1 = uv->uv[0].x;
	x2 = uv->uv[1].x;
	y1 = uv->uv[0].y;
	y2 = uv->uv[2].y - uv->uv[0].y;
	glBegin(GL_QUAD_STRIP);
	for (unsigned i = 0; i < _points.size(); i++) {
		yi = y1 + y2 * ((float)i / (float)_points.size());

		if (i == _points.size() - 1)
			yi = y1;
		glTexCoord2f(x1, yi);
		glVertex3f(_points[i].left.x, _points[i].left.y, DEPTH_FX);
		glTexCoord2f(x2, yi);
		glVertex3f(_points[i].right.x, _points[i].right.y, DEPTH_FX);
	}
	glEnd();
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void bodyLeg::Init(GLrgba c, float stride, float knee_height, float knee_size, float step_radius, float start_angle)
{
	_color = c;
	_knee_size = knee_size;
	_knee_height = knee_height;
	_stride = stride;
	_current_stride = _stride / 3;
	_step_radius = step_radius;
	_angle = start_angle;
	_sound_step = "bump";
	_sprite_knee.Init(SPRITE_KNEE, _knee_size * 1.5f, c);
	_ready = false;
	_extended = false;
}

template <typename T> int sign(T val) {
	return (T(0) < val) - (val < T(0));
}

void bodyLeg::Move(GLvector2 body)
{
	GLvector2   step;
	bool        stomp;
	float       max_depth;

	max_depth = body.y + _knee_height;
	_origin = body;
	_foot.x = body.x + _current_stride;
	_current_floor = CollisionFloor(GLvector2(_foot.x, body.y - 0.1f));
	_current_floor = min(max_depth, _current_floor);
	if (!_ready) {
		_ready = true;
		_foot.y = _current_floor;
		_current_walk = _origin.x;
	}
	if (_extended)
		_current_stride += (_stride - _current_stride) * 0.02f;
	if (_current_walk != _origin.x) {
		float   distance_walked;

		distance_walked = (_current_walk - _origin.x) / _step_radius;
		_angle -= distance_walked * WALK_SPEED;
		_current_walk = _origin.x;
	}
	_foot.y -= (_foot.y - _current_floor) * STEP_UP_INTERPOLATE;
	stomp = false;
	if (_extended)
		step = SpriteMapVectorRotate(GLvector2(0, _step_radius), (int)_angle);
	else
		step = SpriteMapVectorRotate(GLvector2(0, _step_radius), 0);
	if (step.y > 0.0f) { //This means the foot is on the ground
		if (_foot_up) {
			_foot_up = false;
			stomp = true;
		}
		step.y = 0;
	}	else //Foot is arcing over the base
		_foot_up = true;
	_tip = _foot + step;
	_knee_center = _foot + GLvector2(step.x / 3, -_knee_height + step.y);
	_knee_center.x -= _current_stride / 2;
	_sprite_knee.Move(_knee_center);
	_knee.corner[0] = _knee_center - GLvector2(0, -_knee_size);
	_knee.corner[1] = _knee_center - GLvector2(0, _knee_size);
	_knee.corner[2] = _knee_center - GLvector2(-_knee_size, 0);
	_knee.corner[3] = _knee_center - GLvector2(_knee_size, 0);
	_knee_offset = _knee_center - _origin;
	_tip.y = CollisionFloor(GLvector2(_tip.x, _tip.y - 0.5f));
	//if our tip is low then we're stepping off a cliff.  Don't let the foot drop
	//all the way to the floor, since that could be a huge distance. Instead, keep
	//the foot at a reasonable height and then move sideways to find a wall to "grab".
	if (_tip.y > max_depth) {
		_tip.y = _knee_height + _step_radius + _knee_center.y;
		_tip.x = floor(_tip.x) + 0.5f;
		while (WorldCellShape(_tip) == 0 && _stride != 0) {
			_tip.x -= sign(_stride);
			if (_stride<0 && _tip.x > body.x) {
				_tip.x = body.x;
				break;
			}
			if (_stride > 0 && _tip.x < body.x) {
				_tip.x = body.x;
				break;
			}
		}
	}
	//Take all those positions and save them in our arrays for later rendering.
	_triangles[0] = GLvector (_origin.x, _origin.y - _knee_size, 0);
	_triangles[1] = GLvector (_knee.corner[0].x, _knee.corner[0].y, 0);
	_triangles[2] = GLvector (_knee.corner[1].x, _knee.corner[1].y, 0);

	_triangles[3] = GLvector (_origin.x, _origin.y + _knee_size, 0);
	_triangles[4] = GLvector (_origin.x, _origin.y - _knee_size, 0);
	_triangles[5] = GLvector (_knee.corner[1].x, _knee.corner[1].y, 0);

	_triangles[6] = GLvector (_knee.corner[2].x, _knee.corner[2].y, 0);
	_triangles[7] = GLvector (_knee.corner[3].x, _knee.corner[3].y, 0);
	_triangles[8] = GLvector (_tip.x, _tip.y, 0);

	if (stomp) {
		if (AudioValid(_sound_step))
			AudioPlay(_sound_step, _foot);
		ParticleRubble(_tip, 0.05f, 4);
	}
}

void bodyLeg::Update()
{
}

void bodyLeg::Render()
{
	/*
	RenderQuads();
	glFlush();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(0, 0, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	*/
	RenderTriangle (&_triangles[0]);
	RenderTriangle (&_triangles[3]);
	RenderTriangle (&_triangles[6]);
	/*
	glBegin(GL_TRIANGLES);
	glVertex3f(_origin.x, _origin.y - _knee_size, 0);
	glVertex3f(_knee.corner[0].x, _knee.corner[0].y, 0);
	glVertex3f(_knee.corner[1].x, _knee.corner[1].y, 0);

	glVertex3f(_origin.x, _origin.y + _knee_size, 0);
	glVertex3f(_origin.x, _origin.y - _knee_size, 0);
	glVertex3f(_knee.corner[1].x, _knee.corner[1].y, 0);

	glVertex3f(_knee.corner[2].x, _knee.corner[2].y, 0);
	glVertex3f(_knee.corner[3].x, _knee.corner[3].y, 0);
	glVertex3f(_tip.x, _tip.y, 0);
	glEnd();
	*/
	//glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
	_sprite_knee.Render();
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void bodyEye::Init(GLrgba color_out, GLrgba color_in, SpriteEntry eye, SpriteEntry iris, float size)
{
	_color_outer = color_out;
	_color_inner = color_in;
	_size = size;
	_uvmask = SpriteMapLookup(eye);
	_uvlight = SpriteMapLookup(eye);
	_blink = 0;
	_sprite_eye = eye;
	_sprite_iris = iris;
}

void bodyEye::SetColor(GLrgba c)
{
	_color_outer = c;
	_color_inner = (c + GLrgba(1, 1, 1) + GLrgba(1, 1, 1)) / 3;
}

void bodyEye::Move(GLvector2 pos, GLvector2 look, float angle)
{
	_position = pos;
	_angle = angle;
	_look.x = clamp(look.x, -1.0f, 1.0f);
	_look.y = clamp(look.y, -1.0f, 1.0f);
}

void bodyEye::Render()
{
	GLvector2   shift;

	//  glDepthMask (false);
	//RenderQuad (_position, SPRITE_INVALID, GLrgba (), _size*2, _angle, DEPTH_EYES, false);
	RenderQuads();
	return;
	//glDepthMask (true);

	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//If we're blinking, we have to render the eye manually because our vertex shader isn't
	//robust enough to handle partial polys. Note that THIS way isn't robist enough to
	//rotate them, so an eye can't boith blink and rotate.
	if (_blink > 0.0f) {
		float   lid;

		lid = _size * 2 * _blink;
		RenderQuad (_position, _sprite_eye, _color_outer, GLvector2 (_size,_size), _angle, DEPTH_EYES, false, _blink);
	}	else {
		RenderQuad(_position, _sprite_eye, _color_outer, _size * 2, _angle, DEPTH_EYES, false);
		RenderQuads();
	}
	glDepthFunc(GL_EQUAL);
	shift = _look * (_size * 0.5f);
	shift = SpriteMapVectorRotate(shift, (int)_angle);
	shift.y *= -1;
	RenderQuad(shift + _position, _sprite_iris, _color_inner, _size * 2, _angle, DEPTH_EYES, true);
	RenderQuads();

	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void bodyEye::RenderEye ()
{
	if (_blink > 0.0f) {
		float   lid;

		lid = _size * 2 * _blink;
		RenderQuad (_position, _sprite_eye, _color_outer, GLvector2 (_size, _size), _angle, DEPTH_EYES, false, _blink);
	} else {
		RenderQuad (_position, _sprite_eye, _color_outer, _size * 2, _angle, DEPTH_EYES, false);
	}
}

void bodyEye::RenderIris ()
{
	GLvector2   shift;

	shift = _look * (_size * 0.5f);
	shift = SpriteMapVectorRotate (shift, (int)_angle);
	shift.y *= -1;
	RenderQuad (shift + _position, _sprite_iris, _color_inner, _size * 2, _angle, DEPTH_EYES, true);
}