/*-----------------------------------------------------------------------------

  Collision.cpp

  Handles collision between objects and the marching squares that make up the
  level geometry.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "collision.h"
#include "env.h"
#include "world.h"

#define POINT_DENSITY             50.0f
#define MAX_POINTS                10
#define RSIZE                     0.01f
#define TOP_EDGE                  GLvector2 (0.5f, 0.0f)
#define LEFT_EDGE                 GLvector2 (0.0f, 0.5f)
#define RIGHT_EDGE                GLvector2 (1.0f, 0.5f)
#define BOTTOM_EDGE               GLvector2 (0.5f, 1.0f)

static vector<GLvector2>        circle[MAX_POINTS];
static vector<GLvector2>        debug_points;
static int                      frame;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

vector<GLvector2>* get_circle(int points)
{
	points = clamp(points, 4, MAX_POINTS - 1);
	if (!circle[points].empty())
		return &circle[points];
	//Fill in the circle with useful values.
	float     step;

	step = 360 / (float)points;
	for (float angle = 0; angle < 360; angle += step)
		circle[points].push_back(GLvectorFromAngle(angle));
	return &circle[points];
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

float CollisionFloor(GLvector2 point)
{
	int       shape;
	int       steps;
	float     frac;
	float     base;

	shape = 0;
	steps = 0;
	base = floor(point.y);
	frac = point.x - floor(point.x);
	while (steps < 20) {
		base = floor(point.y);
		shape = WorldCellShape(point);
		if (shape == 12)  //Flat floor
			return base + 0.5f;
		if (shape == 8 && frac < 0.5f) //west ramp
			return base + frac + 0.5f;
		if (shape == 13 && frac>0.5f)
			return base + (frac - 0.5f);
		if (shape == 4 && frac > 0.5f)
			return (1.5f + base) - (frac);
		if (shape == 14 && frac < 0.5f)
			return (0.5f + base) - (frac);
		point.y += 1.0f;
		steps++;
	}
	return base;
}

float CollisionCeiling(GLvector2 point)
{
	int       shape;
	int       steps;
	float     frac;
	float     base;

	shape = 0;
	steps = 0;
	base = floor(point.y);
	point.y += 1.0f;
	frac = point.x - floor(point.x);
	while (steps < 20) {
		base = floor(point.y);
		shape = WorldCellShape(point);
		if (shape == 3) //flat ceiling
			return base + 0.5f;
		if (shape == 1 && frac < 0.5f) //west sloping ceiling
			return base + 0.5f - frac;
		if (shape == 11 && frac>0.5f) //west sloping inner corner
			return 1 + base - (frac - 0.5f);
		if (shape == 2 && frac > 0.5f) //East sloping ceiling
			return (base - 0.5f) + (frac);
		if (shape == 7 && frac < 0.5f) //East slope inner corner
			return (0.5f + base) + (frac);
		point.y -= 1.0f;
		steps++;
	}
	return base;
}

//This is used when the player slams into a wall. We look at the normal of
//the wall to see which way they should move to slide along it, as opposed
//to bouncing back.

GLvector2 CollisionSlide(GLvector2 wall, GLvector2 movement)
{
	GLvector2   new_movement;

	new_movement = movement;
	//floor
	if (wall == GLvector2(0, -1)) {
		new_movement = wall.TurnedRight();
		if (movement.x < 0)
			new_movement *= -1;
	}
	//ceiling
	if (wall == GLvector2(0, 1)) {
		new_movement = wall.TurnedRight();
		if (movement.x > 0)
			new_movement *= -1;
	}
	//west wall
	if (wall == GLvector2(1, 0)) {
		new_movement = wall.TurnedRight();
		if (movement.y < 0)
			new_movement *= -1;
	}
	//east wall
	if (wall == GLvector2(-1, 0)) {
		new_movement = wall.TurnedRight();
		if (movement.y > 0)
			new_movement *= -1;
	}
	//ramp /
	if (wall == GLvector2(-0.5f, -0.5f)) {
		new_movement = wall.TurnedRight();
		if (fabs(movement.y) < fabs(movement.x))
			new_movement *= -1;
	}
	//ramp \ other way
	if (wall == GLvector2(0.5f, -0.5f)) {
		new_movement = wall.TurnedRight();
		if (fabs(movement.y) > fabs(movement.x))
			new_movement *= -1;
	}
	//ceiling slope facing west
	if (wall == GLvector2(-0.5f, 0.5f)) {
		new_movement = wall.TurnedRight();
		if (fabs(movement.y) > fabs(movement.x))
			new_movement *= -1;
	}
	//ceiling slope facing east
	if (wall == GLvector2(0.5f, 0.5f)) {
		new_movement = wall.TurnedRight();
		if (fabs(movement.y) < fabs(movement.x))
			new_movement *= -1;
	}
	return new_movement;
}

//This is really the ONLY collision checking in the whole game. (For level
//geometry. All other collision check are just different ways to call this.)
//Normal returns the outward-facing normal of the wall, and depth is how
//far into the wall the given point is.
//This is all based on marching cubes and so walls are always at
//orthoganal angles.

bool Collision(GLvector2 point, GLvector2* normal, float* depth)
{
	int         x, y;
	GLvector2   frac;
	short       shape;
  float       ignore;

  //The calling function might pass NULL for the depth value because they 
  //don't want it. Rather than cluttering up the logic below, we catch it
  //here and aim it someplace safe.
  if (depth == NULL)
    depth = &ignore;
	*depth = 0.0f;
	x = (int)point.x;
	y = (int)point.y;
	frac.x = point.x - floor(point.x);
	frac.y = point.y - floor(point.y);
	shape = WorldCellShape(point);
	switch (shape) {
	case 0:
		return false;
	case 1: //NW corner
		*normal = GLvector2(0.5f, 0.5f);
		*depth = 0.5f - (frac.y + frac.x);
		if (*depth > 0.0f)
			return true;
		return false;
	case 2: //NE corner
		*normal = GLvector2(-0.5f, 0.5f);
		frac.x = 1 - frac.x;
		*depth = 0.5f - (frac.y + frac.x);
		if (*depth > 0.0f)
			return true;
		return false;
	case 3: //north wall
		*normal = GLvector2(0, 1);
		*depth = 0.5f - frac.y;
		if (*depth > 0.0f)
			return true;
		return false;
	case 4: //SE corner
		*normal = GLvector2(-0.5f, -0.5f);
		frac.x = 1 - frac.x;
		frac.y = 1 - frac.y;
		*depth = 0.5f - (frac.y + frac.x);
		if (*depth > 0.0f)
			return true;
		return false;
	case 5: //NW and SE corners
		*normal = GLvector2(0.5f, 0.5f);
		*depth = 0.5f - (frac.y + frac.x);
		if (*depth > 0.0f) {
			*normal = GLvector2(0.5f, 0.5f);
			return true;
		}
		frac.x = 1 - frac.x;
		frac.y = 1 - frac.y;
		*depth = 0.5f - (frac.y + frac.x);
		if (*depth > 0.0f) {
			*normal = GLvector2(-0.5f, -0.5f);
			return true;
		}
		return false;
	case 6: //East wall
		*normal = GLvector2(-1, 0);
		*depth = (frac.x) - 0.5f;
		if (*depth > 0.0f)
			return true;
		return false;
	case 7: //SW gap
		*normal = GLvector2(-0.5f, 0.5f);
		frac.y = 1 - frac.y;
		*depth = (frac.y + frac.x) - 0.5f;
		if (*depth > 0.0f)
			return true;
		return false;
	case 8: //SW corner
		*normal = GLvector2(0.5f, -0.5f);
		frac.y = 1 - frac.y;
		*depth = 0.5f - (frac.y + frac.x);
		if (*depth > 0.0f)
			return true;
		return false;
	case 9: //West wall
		*normal = GLvector2(1, 0);
		*depth = 0.5f - (frac.x);
		if (*depth > 0.0f)
			return true;
		return false;
	case 10: //NE and SW corners
		*normal = GLvector2(0.5f, -0.5f);
		frac.y = 1 - frac.y;
		*depth = 0.5f - (frac.y + frac.x);
		if (*depth > 0.0f) {
			*normal = GLvector2(0.5f, -0.5f);
			return true;
		}
		*depth = (frac.y + frac.x) - 1.5f;
		if (*depth > 0.0f) {
			*normal = GLvector2(-0.5f, 0.5f);
			return true;
		}
		return false;
	case 11: //SE gap
		*normal = GLvector2(0.5f, 0.5f);
		*depth = 1.5f - (frac.y + frac.x);
		if (*depth > 0.0f)
			return true;
		return false;
	case 12://South wall
		*normal = GLvector2(0, -1);
		*depth = (frac.y) - 0.5f;
		if (*depth > 0.0f)
			return true;
		return false;
	case 13: //NE gap
		*normal = GLvector2(0.5f, -0.5f);
		frac.x = 1 - frac.x;
		*depth = (frac.y + frac.x) - 0.5f;
		if (*depth > 0.0f)
			return true;
		return false;
	case 14: //NW gap
		*normal = GLvector2(-0.5f, -0.5f);
		*depth = (frac.y + frac.x) - 0.5f;
		if (*depth > 0.0f)
			return true;
		return false;
	case 15: //Solid
		*normal = GLvector2(-0.0f, -1.0f);
		*depth = 1.0f;
		return true;
	}
	return false;
}

//Simple pass /fail.
bool Collision(GLvector2 point)
{
	GLvector2 unwanted_normal;
	float     unwanted_depth;

	return Collision(point, &unwanted_normal, &unwanted_depth);
}

//This creates a number of points around the given one, at the requested radius.
//The bigger the radius, the more points. It's many times slower, but you can't just
//treat a gigantic boss monster like a single point or it will get into all sorts
//of places it shouldn't.
bool Collision(GLvector2 position, GLvector2* normal, float* depth, float radius)
{
	float               circum;
	int                 points;
	vector<GLvector2>*  circle;

	circum = PI * 2 * radius;
	//We want a point every n units around the circumference
	points = (int)(circum * POINT_DENSITY);
	circle = get_circle(points);
	for (unsigned i = 0; i < circle->size(); i++) {
		GLvector2   pos;

		pos = position + circle->at(i) * radius;
		if (EnvValueb(ENV_BUMP))
			debug_points.push_back(pos);
		if (Collision(pos, normal, depth))
			return true;
	}
	return false;
}

//As above, but we don't care about reflection angles.
bool Collision(GLvector2 position, float radius)
{
	GLvector2 unwanted_normal;
	float     unwanted_depth;

	return Collision(position, &unwanted_normal, &unwanted_depth, radius);
}

//This performs a line-of-sight check between the given points, performing
//checks at the given interval. TRUE if visible, FALSE if obscured.
bool CollisionLos(GLvector2 start, GLvector2 end, float interval)
{
	GLvector2 step;
	GLvector2 offset;
	GLvector2 consider;
	float     distance;
	float     steps;
	float     v;

	offset = end - start;
	distance = offset.Length();
	offset.Normalize();
	offset *= interval;
	steps = floor(distance / interval);
	for (v = 1.0f; v < steps; v += 1.0f) {
		consider = start + (offset * v);
		if (EnvValueb(ENV_BUMP))
			debug_points.push_back(consider);
		if (Collision(consider))
			return false;
	}
	return true;
}

//Not used in production situations. This just renders all the collision checks
//that were performed this frame.
void CollisionRender()
{
	if (!EnvValueb(ENV_BUMP))
		return;
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glColor3f(1, 1, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	for (unsigned i = 0; i < debug_points.size(); i++) {
		glVertex3f(debug_points[i].x - RSIZE, debug_points[i].y - RSIZE, DEPTH_UNITS);
		glVertex3f(debug_points[i].x + RSIZE, debug_points[i].y - RSIZE, DEPTH_UNITS);
		glVertex3f(debug_points[i].x + RSIZE, debug_points[i].y + RSIZE, DEPTH_UNITS);
		glVertex3f(debug_points[i].x - RSIZE, debug_points[i].y + RSIZE, DEPTH_UNITS);
	}
	glEnd();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	//Only clear the list every few frames, so we can SEE the dots.
	frame = (frame + 1) % 5;
	if (!frame)
		debug_points.clear();
}

//For the given cell, return a vector containing all the line segments
//that make up its shape. This is used by the visibility system to generate
//2D "shadows".
bool CollisionLine(GLcoord2 cell, vector<Line2D>& list)
{
	short             shape;
	GLvector2         origin;
	Line2D            line;

	origin = cell;
	shape = WorldCellShape(cell);
	switch (shape) {
	case 0:
	case 15:
		return false;
	case 1:
	case 14://Technically, 1 the opposite way, but we don't care about normals.
		line.start = origin + LEFT_EDGE;
		line.end = origin + TOP_EDGE;
		list.push_back(line);
		return true;
	case 2:
	case 13://Technically, 2 the opposite way, but we don't care about normals.
		line.start = origin + TOP_EDGE;
		line.end = origin + RIGHT_EDGE;
		list.push_back(line);
		return true;
	case 3:
	case 12://Technically, 3 the opposite way, but we don't care about normals.
		line.start = origin + LEFT_EDGE;
		line.end = origin + RIGHT_EDGE;
		list.push_back(line);
		return true;
	case 4:
	case 11://Technically, 4 the opposite way, but we don't care about normals.
		line.start = origin + RIGHT_EDGE;
		line.end = origin + BOTTOM_EDGE;
		list.push_back(line);
		return true;
	case 5:
		line.start = origin + LEFT_EDGE;
		line.end = origin + TOP_EDGE;
		list.push_back(line);
		line.start = origin + RIGHT_EDGE;
		line.end = origin + BOTTOM_EDGE;
		list.push_back(line);
		return true;
	case 6:
	case 9://Technically, 6 the opposite way, but we don't care about normals.
		line.start = origin + TOP_EDGE;
		line.end = origin + BOTTOM_EDGE;
		list.push_back(line);
		return true;
	case 7:
	case 8://Technically, 7 facing inward, but we don't care about normals.
		line.start = origin + LEFT_EDGE;
		line.end = origin + BOTTOM_EDGE;
		list.push_back(line);
		return true;
	case 10://8 and 2 merged
		line.start = origin + BOTTOM_EDGE;
		line.end = origin + LEFT_EDGE;
		list.push_back(line);
		line.start = origin + TOP_EDGE;
		line.end = origin + RIGHT_EDGE;
		list.push_back(line);
		return true;
	}
	return false;
}