/*-----------------------------------------------------------------------------

  Transition.cpp

  This is used when we need to move the camera large distances, as when the
  player respawns. A sudden cut can leave the player confused as to where
  they've gone, so we pan across the space in a way that will hopefully look
  cool with our paralaxing background.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "game.h"

#define TRANSITION_SPEED        50 //How many ms per world unit

static bool       transition_active;
static int        transition_begin;
static GLvector2  transition_camera_begin;
static GLvector2  transition_camera_end;
static GLvector2  transition_camera;
static int        transition_duration;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void TransitionBegin(GLvector2 begin, GLvector2 end)
{
	GLvector2     offset;

	offset = end - begin;
	transition_duration = 250 + (int)(TRANSITION_SPEED * offset.Length());
	transition_camera_begin = begin;
	transition_camera_end = end;
	transition_active = true;
	transition_begin = GameTick();
}

bool TransitionActive ()
{
	return transition_active;
}

void TransitionUpdate()
{
	int       elapsed;
	float     delta;

	if (!transition_active)
		return;
	elapsed = GameTick() - transition_begin;
	if (elapsed < 1000) {
		transition_camera = transition_camera_begin;
		return;
	}
	elapsed -= 1000;
	delta = (float)elapsed / (float)transition_duration;
	delta = syMathScalarCurve(delta);
	transition_camera = Lerp(transition_camera_begin, transition_camera_end, delta);
	if (elapsed > transition_duration) {
		transition_active = false;
	}
}

GLvector2 TransitionCamera()
{
	TransitionUpdate();
	return transition_camera;
}