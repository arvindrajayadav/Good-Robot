/*-----------------------------------------------------------------------------

  Input.cpp

  Tracks keyboard & mouse input

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"
#include "system.h"
#include "input.h"
#include "main.h"

#define MAX_AXIS						6
#define MAX_JOY_BUTTON			12
#define JOY_DEAD_ZONE				10000
#define STICK_MAX           32768

struct Control
{
	bool            down;
	bool            pressed;
	int             value;
	float           valuef;
};

static GLcoord2   mouse;
static Control    control[MAX_KEYS];
static bool       mouselook;
static bool       joystick_active;
static bool       vstick_centered;
static bool       hstick_centered;
static bool       using_joystick;
static int        last_pressed;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

bool is_joybutton (int id)
{
  if (id >= INPUT_JOY_BASE && id < INPUT_JOY_END)
    return true;
  return false;
}

//Convention allows for using thumbstick to navigate menus.
//So we look at the stick wagging up and down and treat these like DPAD presses.
void simulate_dpad(int stick_y)
{
	if (stick_y < -JOY_DEAD_ZONE)
		InputKeyDown(INPUT_JOY_UP);
	else
		InputKeyUp(INPUT_JOY_UP);
	if (stick_y > JOY_DEAD_ZONE)
		InputKeyDown(INPUT_JOY_DOWN);
	else
		InputKeyUp(INPUT_JOY_DOWN);
}

//It's often more convenient to treat the triggers as buttons and not as an axis.
void simulate_trigger(int axis, int position)
{
	if (axis == JOY_TRIGGER_LEFT) {
		if (position > JOY_DEAD_ZONE)
			InputKeyDown (TRIGGER_LEFT);
		else
			InputKeyUp (TRIGGER_LEFT);
	}
	if (axis == JOY_TRIGGER_RIGHT) {
		if (position < -JOY_DEAD_ZONE)
			InputKeyDown (TRIGGER_RIGHT);
		else
			InputKeyUp (TRIGGER_RIGHT);
	}
}

static int translate_key(int id)
{
	if (id < MAX_KEYS && id > 0)
		return id;
	return 0;
}

Control* control_from_id(int id)
{
	if (id < MAX_KEYS && id > 0)
		return &control[id];
	return &control[INPUT_INVALID];
}

Control* control_from_stick(int axis)
{
	if (axis < 0 || axis >= MAX_AXIS)
		return &control[INPUT_INVALID];
	return control_from_id(INPUT_JOY_LSTICK_X + axis);
}

/*-----------------------------------------------------------------------------
Joystick
-----------------------------------------------------------------------------*/

void InputJoyDown(int id)
{
	if (id < 0 || id >= XBOX_MAX)
		return;
	InputKeyDown(INPUT_JOY_BASE + id);
}

void InputJoyUp(int id)
{
	if (id < 0 || id >= MAX_JOY_BUTTON)
		return;
	InputKeyUp(INPUT_JOY_BASE + id);
}

bool InputJoystickActive()
{
	return joystick_active;
}

bool InputAxisNeutral(int axis)
{
	Control* c = control_from_stick(axis);

	if (abs(c->value) > JOY_DEAD_ZONE)
		return false;
	return true;
}

void InputAxisMove(int axis, int position)
{
	Control* c = control_from_stick(axis);

	if (axis < 0 || axis >= MAX_AXIS)
		return;
	if (!joystick_active)
		InputClearState ();
	joystick_active = true;
	/*
	if (axis == JOY_TRIGGER_LEFT)
		simulate_dpad(position);
		*/
	if (axis == JOY_LEFT_STICK_Y)
		simulate_dpad (position);
	if (axis == JOY_TRIGGER_RIGHT)
		simulate_trigger(axis, position);
	if (axis == JOY_TRIGGER_LEFT)
		simulate_trigger (axis, position);
	c->value = position;
	c->valuef = (float)position / (float)STICK_MAX;
}

int InputAxis(int axis)
{
	Control* c = control_from_stick(axis);
	return c->value;
}

float InputAxisf(int axis)
{
	Control* c = control_from_stick(axis);
	return c->valuef;
}

/*-----------------------------------------------------------------------------
Keyboard
-----------------------------------------------------------------------------*/

void InputKeyDown(int id)
{
	Control* c = control_from_id(id);

  joystick_active = is_joybutton (id);
	if (!c->down)
		c->pressed = true;
	last_pressed = id;
	c->down = true;
	c->value = STICK_MAX;
	c->valuef = (float)STICK_MAX;
}

void InputKeyUp(int id)
{
	Control* c = control_from_id(id);

	//The system spams us with "keyup" events for mouse. I don't know why, but
	//removing it will break controls for some. So ignore this event.
	if (id != INPUT_MOUSE_MOVED)
		joystick_active = is_joybutton (id);
	c->down = false;
	c->value = 0;
	c->valuef = 0.0f;
}

bool InputKeyState(int id)
{
	return control_from_id(id)->down;
}

bool InputKeyPressed(int id)
{
	Control* c = control_from_id(id);
	bool      val;

	val = c->pressed;
	c->pressed = false;
	return val;
}

/*-----------------------------------------------------------------------------
Mouse
-----------------------------------------------------------------------------*/

bool InputMouselook()
{
	return mouselook;
}

void InputMouselookSet(bool val)
{
	mouselook = val;
}

void InputMouseMove(int x, int y)
{
	if (joystick_active)
		InputClearState ();
	joystick_active = false;
	mouse.x += x;
	mouse.y += y;
}

GLcoord2 InputMouseMovement()
{
	GLcoord2  delta = mouse;

	mouse = GLcoord2(0, 0);
	return delta;
}

GLcoord2 InputMousePosition()
{
	return SystemMouse();
}

/*-----------------------------------------------------------------------------
Misc
-----------------------------------------------------------------------------*/

int InputLastPressed()
{
	int   result;

	result = last_pressed;
	last_pressed = 0;
	return result;
}

bool InputAnyKeyPressed()
{
	return (last_pressed != 0);
}

void InputClearState ()
{
	for (int i = 0; i < MAX_KEYS; i++) {
		control[i].down = false;
		control[i].pressed = false;
	}
}