#ifndef INPUT_H
#define INPUT_H

enum
{
	XBOX_A,
	XBOX_B,
	XBOX_X,
	XBOX_Y,
	XBOX_LB,
	XBOX_RB,
	XBOX_BACK,
	XBOX_START,
	XBOX_LSTICK_X,
	XBOX_LSTICK_Y,
	XBOX_RSTICK_X,
	XBOX_RSTICK_Y,
	XBOX_MAX
};

#define JOY_LEFT_STICK_X      0
#define JOY_LEFT_STICK_Y      1

#define JOY_RIGHT_STICK_X     3
#define JOY_RIGHT_STICK_Y     4
#define JOY_TRIGGER_LEFT			2
#define JOY_TRIGGER_RIGHT			5

#define INPUT_INVALID         0

#define INPUT_JOY_BASE        450
#define INPUT_JOY_A           (INPUT_JOY_BASE + XBOX_A)
#define INPUT_JOY_B           (INPUT_JOY_BASE + XBOX_B)
#define INPUT_JOY_X           (INPUT_JOY_BASE + XBOX_X)
#define INPUT_JOY_Y           (INPUT_JOY_BASE + XBOX_Y)
#define INPUT_JOY_LB          (INPUT_JOY_BASE + XBOX_LB)  //454
#define INPUT_JOY_RB          (INPUT_JOY_BASE + XBOX_RB)  //455
#define INPUT_JOY_BACK        (INPUT_JOY_BASE + XBOX_BACK)
#define INPUT_JOY_START       (INPUT_JOY_BASE + XBOX_START)
#define INPUT_JOY_LSTICK_X    (INPUT_JOY_BASE + XBOX_LSTICK_X)
#define INPUT_JOY_LSTICK_Y    (INPUT_JOY_BASE + XBOX_LSTICK_Y)
#define INPUT_JOY_RSTICK_X    (INPUT_JOY_BASE + XBOX_RSTICK_X)
#define INPUT_JOY_RSTICK_Y    (INPUT_JOY_BASE + XBOX_RSTICK_Y)
#define INPUT_JOY_LTRIGGER    (INPUT_JOY_BASE + XBOX_MAX+1)  //463
#define INPUT_JOY_RTRIGGER    (INPUT_JOY_BASE + XBOX_MAX+2)  //464
#define INPUT_JOY_DPAD_UP     (INPUT_JOY_BASE + XBOX_MAX+3)
#define INPUT_JOY_DPAD_DOWN   (INPUT_JOY_BASE + XBOX_MAX+4)
#define INPUT_JOY_DPAD_LEFT   (INPUT_JOY_BASE + XBOX_MAX+5)
#define INPUT_JOY_DPAD_RIGHT  (INPUT_JOY_BASE + XBOX_MAX+6)
//These next two keys are linked to both stick movement AND D-Pad
#define INPUT_JOY_UP          (INPUT_JOY_BASE + XBOX_MAX+7)
#define INPUT_JOY_DOWN        (INPUT_JOY_BASE + XBOX_MAX+8)
#define INPUT_JOY_END         (INPUT_JOY_BASE + XBOX_MAX+9)

#define TRIGGER_LEFT          504
#define TRIGGER_RIGHT         505
#define INPUT_LMB             508
#define INPUT_RMB             509
#define INPUT_MWHEEL_UP       510
#define INPUT_MWHEEL_DOWN     511
#define INPUT_MOUSE_MOVED     512
#define MAX_KEYS              513

void			InputClearState();
void      InputAxisMove(int axis, int position);
int       InputAxis(int axis);
bool      InputAxisNeutral(int axis);
float     InputAxisf(int axis);
bool      InputJoystickActive();
void      InputJoyDown(int id);
void      InputJoyUp(int id);

void      InputKeyDown(int id);
void      InputKeyUp(int id);
bool      InputKeyState(int id);
bool      InputKeyPressed(int id);
int       InputLastPressed();
bool      InputAnyKeyPressed();

bool      InputMouselook();
void      InputMouselookSet(bool val);
void      InputMouseMove(int x, int y);
GLcoord2  InputMousePosition();
GLcoord2  InputMouseMovement();

#endif // INPUT_H
