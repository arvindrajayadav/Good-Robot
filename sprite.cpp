/*-----------------------------------------------------------------------------

  Sprite.cpp

  Good Robot
  (c) 2013 Shamus Young

  A class for moving single quads around.

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "bodyparts.h"
#include "game.h"
#include "random.h"
#include "render.h"
#include "sprite.h"
#include "texture.h"
#include "world.h"

#define BODY_SCALING    2

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void SpriteUnit::Init()
{
	_enabled = false;
}

bool SpriteUnit::Collide(GLvector2 hit)
{
	GLvector2   offset;
	GLuvFrame*  body;
	GLvector2   uv;
	//GLcoord2    pixel;

	if (!_enabled)
		return false;
	//Get the relative position of the attack to our sprite
	offset = hit - _position;
	//Rotate it into local space
	if (_angle)
		offset = SpriteMapVectorRotate(offset, 180 + (int)_angle);
	//adjust so it's relative to the upper corner.
	offset += GLvector2(_size, _size) * 0.5f;
	if (offset.x<0 || offset.x>_size)
		return false;
	if (offset.y<0 || offset.y>_size)
		return false;
	_tick_stop_shaking = 0;
	//Now convert it into texture coords.
	offset /= _size;
	body = SpriteMapLookup(_sprite_entry);
	uv.x = Lerp(body->uv[0].x, body->uv[1].x, offset.x);
	uv.y = Lerp(body->uv[0].y, body->uv[3].y, offset.y);
	return SpriteMapAlpha(uv);
}

//This allows artists to select specific entries from the sprite sheet
//rather than referring to them through the sprite map.
void SpriteUnit::Init (GLcoord2 sprite, float body_size, GLrgba body_color, SpriteEntry eye, SpriteEntry iris, GLrgba eye_color, float eye_size, GLvector2 eyepos)
{
	_size = body_size * BODY_SCALING;
	_sprite_entry = SPRITE_INVALID;
	_eye.Init (body_color, eye_color, eye, iris, eye_size * body_size);
	_color = body_color;
	_eye_offset = eyepos * body_size;
	_use_eye = true;
	_enabled = true;
	_tick_stop_shaking = 0;
	_angle = 0;
	_position = GLvector2();
	_look = GLvector2();
	_quad = SpriteMapQuad((int)_angle);
}

void SpriteUnit::Init(GLcoord2 sprite, float body_size, GLrgba body_color)
{
	_size = body_size * BODY_SCALING;
	_sprite_entry = SPRITE_INVALID;
	_color = body_color;
	_use_eye = false;
	_enabled = true;
	_angle = 0;
	_tick_stop_shaking = 0;
	_position = GLvector2();
	_look = GLvector2();
	_quad = SpriteMapQuad((int)_angle);
}

void SpriteUnit::Init (SpriteEntry index, float body_size, GLrgba body_color, SpriteEntry eye, SpriteEntry iris, GLrgba eye_color, float eye_size, GLvector2 eyepos)
{
	_size = body_size * BODY_SCALING;
	_sprite_entry = index;
	_eye.Init(body_color, eye_color, eye, iris, eye_size * body_size);
	_color = body_color;
	_eye_offset = eyepos * body_size;
	_use_eye = true;
	_enabled = true;
	_angle = 0;
	_tick_stop_shaking = 0;
	_position = GLvector2();
	_look = GLvector2();
	_quad = SpriteMapQuad((int)_angle);
}

void SpriteUnit::Init(SpriteEntry index, float body_size, GLrgba body_color)
{
	_size = body_size * BODY_SCALING;
	_sprite_entry = index;
	_color = body_color;
	_use_eye = false;
	_enabled = true;
	_angle = 0;
	_tick_stop_shaking = 0;
	_position = GLvector2();
	_look = GLvector2();
	_quad = SpriteMapQuad((int)_angle);
}

void SpriteUnit::Move(GLvector2 position, float angle)
{
	if (_angle != angle) {
		_angle = AngleLimit(angle);
		_quad = SpriteMapQuad((int)_angle);
	}
	_position = position;
	if (_use_eye)
		_eye.Move(_position + _eye_offset, _look, angle);
}

void SpriteUnit::Look(float pos)
{
	_look = GLvector2(pos, 0);
}

void SpriteUnit::Look(GLvector2 pos)
{
	_look = pos;
}

void SpriteUnit::Render ()
{
	if (!_enabled)
		return;

	if (_tick_stop_shaking > 0) {
		GLvector2	shake = GLvector2 (RandomFloat (), RandomFloat ()) - GLvector2 (0.5f, 0.5f);
		GLrgba		color;

		color = _color;
		shake *= _size * 0.1f;
		if (GameTick () > _tick_stop_shaking)
			_tick_stop_shaking = 0;
		if ((GameTick () / 100) % 2)
			color = GLrgba ();
		RenderQuad (_position + shake, _sprite_entry, color, _size, (float)_angle, DEPTH_UNITS, false);
	} else 
		RenderQuad (_position, _sprite_entry, _color, _size, (float)_angle, DEPTH_UNITS, false);
}

void SpriteUnit::RenderEye ()
{
	if (!_enabled || !_use_eye)
		return;
	_eye.RenderEye ();
}

void SpriteUnit::RenderIris ()
{
	if (!_enabled || !_use_eye)
		return;
	_eye.RenderIris ();
}


void SpriteUnit::Blink(float ammount)
{
	_eye.Blink(ammount);
}

void SpriteUnit::SetColor(GLrgba c)
{
	_color = c;
	_eye.SetColor(c);
}

void SpriteUnit::Shake (int stop_shaking)
{
	_tick_stop_shaking = stop_shaking;
}


/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void SpriteBox::ColorSet(GLrgba color)
{
	_color = color;
	_use_color = true;
}

void SpriteBox::Init(GLvector2 pos, float size, float angle, int sprite_group, int sprite_index)
{
	_quad = SpriteMapQuad((int)angle);
	for (unsigned i = 0; i < 4; i++) {
		_quad.corner[i] *= size;
		_quad.corner[i] += pos;
	}
	_use_color = false;
	_uv = *SpriteMapLookup(sprite_group, sprite_index);
}

void SpriteBox::Render()
{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (_use_color)
		glColor4fv(&_color.red);
	glBegin(GL_QUADS);
	for (unsigned i = 0; i < 4; i++) {
		glTexCoord2fv(&_uv.uv[i].x);
		glVertex3f(_quad.corner[i].x, _quad.corner[i].y, DEPTH_UNITS);
	}
	glEnd();
}