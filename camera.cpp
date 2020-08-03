/*-----------------------------------------------------------------------------

Camera.cpp

This handles the camera position. It chases after the player, making sure to
keep them in the center of the screen but avoiding abrupt movements as much
as possible. (Because jerky camera movement sucks.)

Good Robot
(c) 2015 Shamus Young

-----------------------------------------------------------------------------*/

#include "master.h"

#include "env.h"
#include "game.h"
#include "input.h"
#include "player.h"
#include "random.h"
#include "render.h"
#include "world.h"

#define CAMERA_MAX_DRIFT      0.50f
#define CAMERA_CHASE_SPEED    0.05f
#define CAMERA_ZOOM_SPEED     0.02f
#define SHAKE                 0.25f
#define SHAKE_FALLOFF					0.95f
#define TRANSITION_SPEED      50 //How many ms per world unit

static bool               transition_active;
static int                transition_begin;
static GLvector2          transition_camera_begin;
static GLvector2          transition_camera_end;
static GLvector2          transition_camera;
static int                transition_duration;
static float              debug_camera_adjust;
static GLvector           camera_current;
static GLvector           camera_desired;
static GLvector2          camera_moved;
static GLvector           camera_shake;
static float              shake_power;
static float              zoom;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

//This tells the camera to slide from its current position to the given one.
//The time taken depends on the distance it will travel.
void CameraTransition(GLvector2 end)
{
	GLvector2     offset;
	GLvector2     begin;

	begin.x = camera_current.x;
	begin.y = camera_current.y;
	offset = end - begin;
	transition_duration = 250 + (int)(TRANSITION_SPEED * offset.Length());
	transition_camera_begin = begin;
	transition_camera_end = end;
	transition_active = true;
	transition_begin = GameTick();
}

void CameraShake(float strength)
{
	shake_power = max (shake_power, strength);
	shake_power = min (shake_power, Env ().screen_shake_max);
}

void CameraUpdate()
{
	GLvector    change;
	GLvector2   old_camera;
	GLvector2   position;

	if (!GameActive())
		return;
	if (EnvValueb(ENV_CHEATS)) {
		if (InputKeyState (SDLK_LCTRL))
			return;
		if (InputKeyPressed (SDLK_PAGEDOWN))
			debug_camera_adjust += 0.5f;
		if (InputKeyPressed (SDLK_PAGEUP))
			debug_camera_adjust -= 0.5f;
		if (InputKeyPressed (SDLK_r))
			debug_camera_adjust = 0;
	}
	old_camera = GLvector2(camera_current.x, camera_current.y);
	position = PlayerPosition();
	zoom = Player()->VisionRadius();
	//Pull the camera back if we're outside of the safe screen at the map start.
	if (!WorldIsCalm())
		zoom += 0.5f;
	//Pull the camera back a bit if we're fighting a boss.
	if (WorldBossGet())
		zoom += 1.0f;

	//Have camera chase after player.
	camera_desired.x = position.x;
	camera_desired.y = position.y;
	camera_desired.z = zoom;
	if (debug_camera_adjust < 0)
		camera_desired.z /= (-debug_camera_adjust) + 1;
	else if (debug_camera_adjust > 0)
		camera_desired.z *= debug_camera_adjust + 1;
	//Get the boundary of current level. Keep the camera from peering past the edges.
	GLbbox2 world = WorldBounds();
	//Turn off camera bounds for testing.
	if (!EnvValueb(ENV_OCCLUDE)) {
		world.pmin = GLvector2(-999, -999);
		world.pmax = GLvector2(999, 999);
	}
	world.pmin.x += camera_desired.z*1.55f;
	world.pmax.x -= camera_desired.z*1.55f;
	world.pmin.y += camera_desired.z*(1.55f / RenderAspect());
	world.pmax.y -= camera_desired.z*(1.55f / RenderAspect());

	camera_desired.x = clamp(camera_desired.x, world.pmin.x - 1, world.pmax.x + 1);
	camera_desired.y = clamp(camera_desired.y, world.pmin.y - 1, world.pmax.y + 1);
	change = (camera_desired - camera_current);
	camera_current.x += change.x * CAMERA_CHASE_SPEED;
	camera_current.y += change.y * CAMERA_CHASE_SPEED;
	camera_current.z += change.z * CAMERA_ZOOM_SPEED;
	camera_current.x = clamp(camera_current.x, camera_desired.x - CAMERA_MAX_DRIFT, camera_desired.x + CAMERA_MAX_DRIFT);
	camera_current.y = clamp(camera_current.y, camera_desired.y - CAMERA_MAX_DRIFT, camera_desired.y + CAMERA_MAX_DRIFT);
	camera_moved = GLvector2(camera_current.x, camera_current.y) - old_camera;
	if (shake_power > 0) {
		camera_shake.x = (RandomFloat () - 0.5f) * SHAKE * shake_power;
		camera_shake.y = (RandomFloat () - 0.5f) * SHAKE * shake_power;
		shake_power *= SHAKE_FALLOFF;
		if (shake_power < 0.001)
			shake_power = 0;
	} else
		camera_shake = GLvector ();
}

void CameraInit(GLvector pos)
{
	camera_current = pos;
}

GLvector CameraPosition() { return camera_current + camera_shake; }
GLvector2 CameraPosition2D () { return GLvector2 (camera_current.x + camera_shake.x, camera_current.y + camera_shake.y); }
GLvector2 CameraMoved() { return camera_moved; }