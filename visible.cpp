/*-----------------------------------------------------------------------------

  Visible.cpp

  This handles occlusion and projected shadows.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "bodyparts.h"
#include "camera.h"
#include "collision.h"
#include "env.h"
#include "entity.h"
#include "hud.h"
#include "player.h"
#include "render.h"
#include "world.h"

static vector<Line2D>   lines;
static vector<GLquad>   quads;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

//Take the given point and shove it away from the player.
static GLvector2 extrude(GLvector2 origin, GLvector2 point)
{
	GLvector2   offset;

	offset = point - origin;
	offset.Normalize();
	offset *= (40);
	return point + offset;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void VisibleInit()
{
}

void VisibleUpdate()
{
	GLcoord2        cell_camera;
	GLcoord2        cell;
	GLvector2       player;
	int             radius;
	GLvector        camera;

	lines.clear();
	quads.clear();
	player = PlayerPosition();
	camera = CameraPosition();
	cell_camera = GLcoord2((int)camera.x, (int)camera.y);
	radius = (int)camera.z * 2;
  radius = clamp (radius, 2, 16);
	cell_camera -= GLcoord2(radius, radius);
	for (int x = 0; x < radius * 2; x++) {
		for (int y = 0; y < radius * 2; y++) {
			cell = cell_camera + GLcoord2(x, y);
			CollisionLine(cell, lines);
		}
	}
  for (unsigned i = 0; i < EntityDeviceCount (); i++) {
    if (EntityDeviceFromId (i)->Type () != FX_DOOR)
      continue;
    fxDoor*   d = (fxDoor*)EntityDeviceFromId (i);
    lines.push_back (d->Line ());
  }
	for (unsigned i = 0; i < lines.size(); i++) {
		GLquad    q;

		q.corner[0] = lines[i].start;
		q.corner[1] = lines[i].end;
		q.corner[2] = extrude(player, lines[i].end);
		q.corner[3] = extrude(player, lines[i].start);
		quads.push_back(q);
	}
	if (EnvValueb(ENV_LOS))
		HudMessage(StringSprintf ("LOS wall segments: %d", quads.size()));
}

void VisibleInvert(bool invert)
{ 
	if (invert)
		glStencilFunc(GL_EQUAL, 1, 0xFF);
	else
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
}

void VisibleRenderCone(float intensity, float depth)
{
	GLvector    origin;
	GLvector2   offset;
	GLuvFrame*  uv;
	float       angle;
	GLquad      q;
	GLvector    corner1, corner2;
	GLrgba      light;
	float       radius;

	if (PlayerIgnore())
		return;
	glEnable (GL_BLEND);
	glBlendFunc (GL_ONE, GL_ONE);
	glEnable (GL_TEXTURE_2D);
  glBindTexture (GL_TEXTURE_2D, SpriteMapTexture ());
  if (EnvValueb (ENV_SHADOWS))
    glEnable (GL_STENCIL_TEST);
  glDepthMask (false);
	glDisable (GL_DEPTH_TEST);

	radius = Player()->VisionRadius() * 7;
	light = WorldLampColor()*intensity;
	glColor4fv(&light.red);
	origin = PlayerHead();
	offset.x = PlayerAim().x - origin.x;
  offset.y = PlayerAim ().y - origin.y;
	angle = offset.Angle();
	q = SpriteMapQuad((int)angle);

	uv = SpriteMapLookup(SPRITE_FADE);
	corner1 = origin + q.corner[0] * radius;
  corner2 = origin + q.corner[1] * radius;
  origin.z = depth;
  corner1.z = depth;
  corner2.z = depth;

	glBegin(GL_TRIANGLES);
	glTexCoord2fv(&uv->uv[0].x);
	glVertex3fv(&origin.x);
	glTexCoord2fv(&uv->uv[3].x);
	glVertex3fv(&corner1.x);
	glTexCoord2fv(&uv->uv[3].x);
	glVertex3fv(&corner2.x);
	glEnd();
	glDepthMask(true);
	glEnable (GL_DEPTH_TEST);
}

void VisibleRender()
{
	//Draw our extruded quads onto the stencil buffer WITHOUT drawing to screen.
	glBindTexture(GL_TEXTURE_2D, 0);
	glColorMask(false, false, false, false);
	glDepthMask(false);
	glBegin(GL_QUADS);
	for (unsigned i = 0; i < quads.size(); i++) {
		for (unsigned j = 0; j < 4; j++)
			glVertex3f(quads[i].corner[j].x, quads[i].corner[j].y, 0.16f);
	}
	glEnd();
	glColorMask(true, true, true, true);
	glDepthMask(true);
}

void VisibleRenderLOS ()
{
	//Show the line segments and extruded quads in debug view.
	glDisable (GL_STENCIL_TEST);
	glDisable (GL_DEPTH_TEST);
	glColor3f (1, 1, 0);
	glBindTexture (GL_TEXTURE_2D, 0);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin (GL_LINES);
	for (unsigned i = 0; i < lines.size (); i++) {
		GLrgba c = GLrgbaUnique (i);
		glColor3fv (&c.red);
		glVertex3f (lines[i].start.x, lines[i].start.y, 0.1f);
		glVertex3f (lines[i].end.x, lines[i].end.y, 0.1f);
	}
	glEnd ();
	glDepthMask (false);
	glEnable (GL_DEPTH_TEST);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f (0.5f, 0.0f, 0.0f, 0.2f);
	glBegin (GL_QUADS);
	for (unsigned i = 0; i < quads.size (); i++) {
		for (unsigned j = 0; j < 4; j++)
			glVertex3f (quads[i].corner[j].x, quads[i].corner[j].y, 0.16f);
	}
	glEnd ();
	glEnable (GL_STENCIL_TEST);
	glDepthMask (true);

}