/*-----------------------------------------------------------------------------

  System.cpp

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "env.h"
#include "font.h"
#include "ini.h"
#include "input.h"
#include "main.h"
#include "menu.h"
#include "render.h"
#include "system.h"
#include "menu.h"
#include "game.h"

#define SETTINGS_INI     "settings.ini"

class Controller
{
public:
	int                     _index;
	int                     _joystick_id;
	SDL_GameController*     _handle_device;
	SDL_Haptic*             _handle_haptic;
	bool                    _is_haptic;
	bool                    _is_active;

	int                     JoystickId() { return _joystick_id; }
	void                    Init(int index);
	void                    Rumble(float strength, int ms);
};

void Controller::Rumble(float strength, int ms)
{
	if (!_is_active || !_is_haptic)
		return;
	SDL_HapticRumblePlay(_handle_haptic, strength, ms);
}

void Controller::Init(int index)
{
	_index = index;
	_is_active = false;
	_is_haptic = false;
	_handle_device = nullptr;
	_handle_haptic = nullptr;

	if (!SDL_IsGameController(index))
		return;
	_handle_device = SDL_GameControllerOpen(_index);
	_is_active = true;
	Console("Found device: %s", SDL_GameControllerName(_handle_device));
	SDL_Joystick* j = SDL_GameControllerGetJoystick(_handle_device);
	_joystick_id = SDL_JoystickInstanceID(j);
	_handle_haptic = SDL_HapticOpenFromJoystick(j);
	if (_handle_haptic != nullptr) {
		if (SDL_HapticRumbleInit(_handle_haptic) == 0) {
			_is_haptic = true;
		}
	}
};

static bool               lmb;
static bool               mmb;
static long               last_update;
static SDL_Window *				sdl_window;
static SDL_Haptic*				sdl_haptic;
static GLcoord2           mouse_pos;
static GLcoord2           screen_size;
static GLcoord2           screen_native;
static unsigned           next_joystick_check;
static vector<GLcoord2>   resolution_size;
static string             resolution_list;
static int                resolution_index;
static int                current_controller;
static vector<Controller> controllers;
static vector<int>        joystick_map;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void controllers_enumerate()
{
	//Find all controllers plugged into the system.
	controllers.clear();
	joystick_map.resize(SDL_NumJoysticks(), 0);
	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		Controller c;
		c.Init(i);
		controllers.push_back(c);
		joystick_map[c.JoystickId()] = i;
	}
	Console("Found %d controllers.", controllers.size());
}

static void ini_update()
{
	iniFile   ini;

	ini.Open(SystemConfigFile());
	ini.IntSet("Window", "Width", screen_size.x);
	ini.IntSet("Window", "Height", screen_size.y);
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void SystemCreate(bool valid_check)
{
	int         flags;
	GLcoord2    win_size;
	bool        fullscreen;
	bool        skip_sanity;
	iniFile     ini;

	ini.Open(SystemConfigFile());
	win_size.x = max(ini.IntGet("Window", "Width"), 640);
	win_size.y = max(ini.IntGet("Window", "Height"), 480);
	skip_sanity = ini.BoolGet("Window", "SkipSanity");
	fullscreen = ini.BoolGet("Settings", "Fullscreen");
	EnvValueSetb(ENV_FULLSCREEN, fullscreen);
	flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;

	if (fullscreen)
		flags |= SDL_WINDOW_FULLSCREEN;
	if (!skip_sanity) {
		//Figure out which pre-set resolution we're in
		bool    valid = false;

		for (unsigned i = 0; i < resolution_size.size(); i++) {
			if (resolution_size[i] == win_size) {
				valid = true;
				resolution_index = i;
				break;
			}
		}
		//If fullscreen, make sure the proposed mode is valid.
		//If the size isn't a valid resolution, then use zero,
		//which will use native resolution.
		if (fullscreen && !valid && !skip_sanity)
			win_size = screen_native;
	}
	sdl_window = SDL_CreateWindow(WINDOWTITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, win_size.x, win_size.y, flags);
	if (!sdl_window)
		Console("Unable to set video mode: %s", SDL_GetError());
	SDL_SetWindowTitle(sdl_window, WINDOWTITLE);
	last_update = SDL_GetTicks();
	SDL_GL_CreateContext(sdl_window);
	SDL_GetWindowSize(sdl_window, &screen_size.x, &screen_size.y);
	if (EnvValueb(ENV_FULLSCREEN))
		SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN);
	else
		SDL_SetWindowFullscreen(sdl_window, 0);
	RenderResize(screen_size.x, screen_size.y);
	SystemGrab();
}

void SystemSizeWindow()
{
	GLcoord2    win_size;
	iniFile     ini;

	ini.Open(SystemConfigFile());
	win_size.x = max(ini.IntGet("Window", "Width"), 640);
	win_size.y = max(ini.IntGet("Window", "Height"), 480);

	if (EnvValueb(ENV_FULLSCREEN))
		SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN);
	else
		SDL_SetWindowFullscreen(sdl_window, 0);

	SDL_SetWindowSize(sdl_window, win_size.x, win_size.y);
	SDL_SetWindowPosition(sdl_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_GetWindowSize(sdl_window, &screen_size.x, &screen_size.y);

	RenderResize(screen_size.x, screen_size.y);
	FontResize();
	SystemGrab();
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void SystemInit()
{
	GLenum                err;
	int										monitor = 0;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		Console("Unable to initialize SDL: %s\n", SDL_GetError());
		return;
	}
	Console("SdlInit: SDL ready.");
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	//ANGLE_platform_angle_d3d(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE);
	// Get available fullscreen/hardware modes
	resolution_list = "";
	for (int i = 0; i < SDL_GetNumDisplayModes(monitor); i++) {
		SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0 };
		SDL_GetDisplayMode(monitor, i, &mode);

		resolution_size.push_back(GLcoord2(mode.w, mode.h));
		resolution_list = StringSprintf("%dx%d\n", mode.w, mode.h) + resolution_list;
	}
	SystemCreate(true);
	sdl_haptic = SDL_HapticOpen(0);
	// Initialize simple rumble
	if (sdl_haptic)
		SDL_HapticRumbleInit(sdl_haptic);
	err = glewInit();
	if (GLEW_OK != err)  // glewInit failed, something is seriously wrong.
		Console("Error: %s", glewGetErrorString(err));
	else
		Console("SdlInit: glew ready.");
}

string SystemResolutionList()
{
	return resolution_list;
}

int SystemResolutionIndex()
{
	return resolution_index;
}

string SystemConfigFile()
{
	return SystemSavePath() + SETTINGS_INI;
}

void SystemResolutionSet(int index)
{
	resolution_index = index;
	iniFile     ini;

	ini.Open(SystemConfigFile());
	ini.IntSet("Window", "Width", resolution_size[index].x);
	ini.IntSet("Window", "Height", resolution_size[index].y);
	Console("Changing to mode %d: %d x %d", index, resolution_size[index].x, resolution_size[index].y);
	SystemSizeWindow();
	//Close the menu, since all the elements will be in wrong size and place now.
	MenuOpen(MENU_NONE);
}

GLcoord2	SystemResolution(int i) { return resolution_size[i]; }
int				SystemResolutions() { return resolution_size.size(); }

void SystemTerm()
{
	SDL_Quit();
}

void SystemSwapBuffers()
{
	SDL_GL_SwapWindow(sdl_window);
}

void SystemRumble(float strength, int time_ms)
{
	if (current_controller < 0 || current_controller >= controllers.size())
		return;
	controllers[current_controller].Rumble(strength, time_ms);
	/*
	  if (sdl_haptic == nullptr)
		  return;
	  SDL_HapticRumblePlay (sdl_haptic, strength, time_ms);
	*/
}

void SystemUpdate()
{
	SDL_Event event;

	//joystick_check();
	//Why is this here? "Mouse up" isn't a key. Why does removing this break the game?
	InputKeyUp(INPUT_MOUSE_MOVED);
	while (SDL_PollEvent(&event)) {
		//Doing this here to avoid copy pasting in every case
		switch (event.type) {
		case SDL_MOUSEMOTION:
			current_controller = -1; //Don't shake an attached controller if playing with mouse.
			InputKeyDown(INPUT_MOUSE_MOVED);
			if (!ConsoleIsOpen() && !MenuIsOpen())
				InputMouseMove(event.motion.yrel, event.motion.xrel);
			mouse_pos.x = event.motion.x;
			mouse_pos.y = event.motion.y;
			break;
		case SDL_QUIT:
			//if (GameRunning())
			//	GameSave();
			MainQuit();
			break;
		case SDL_JOYHATMOTION:
			if (event.jhat.value &  SDL_HAT_UP)
				InputKeyDown(INPUT_JOY_DPAD_UP);
			else
				InputKeyUp(INPUT_JOY_DPAD_UP);
			if (event.jhat.value &  SDL_HAT_DOWN)
				InputKeyDown(INPUT_JOY_DPAD_DOWN);
			else
				InputKeyUp(INPUT_JOY_DPAD_DOWN);
			if (event.jhat.value &  SDL_HAT_RIGHT)
				InputKeyDown(INPUT_JOY_DPAD_RIGHT);
			else
				InputKeyUp(INPUT_JOY_DPAD_RIGHT);
			if (event.jhat.value &  SDL_HAT_LEFT)
				InputKeyDown(INPUT_JOY_DPAD_LEFT);
			else
				InputKeyUp(INPUT_JOY_DPAD_LEFT);
			break;
		case SDL_JOYAXISMOTION:
			current_controller = joystick_map[event.jaxis.which];
			InputAxisMove(event.jaxis.axis, event.jaxis.value);
			break;
		case SDL_JOYBUTTONDOWN:
			current_controller = joystick_map[event.jaxis.which];
			InputJoyDown(event.jbutton.button);
			break;
		case SDL_JOYBUTTONUP:
			current_controller = joystick_map[event.jaxis.which];
			InputJoyUp(event.jbutton.button);
			break;
		case SDL_KEYDOWN:
#ifdef _DEBUG
			if (event.key.keysym.sym == SDLK_F12)
				MainQuit();
#endif
			if ((event.key.keysym.mod & KMOD_ALT)) {
				if (event.key.keysym.sym == SDLK_F4) {
					MainQuit();
				}
				else if (event.key.keysym.sym == SDLK_RETURN) {
					EnvValueSetb(ENV_FULLSCREEN, !EnvValueb(ENV_FULLSCREEN));
					SystemCreate(true);
					MenuResize();
					return;
				}
			}
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				SystemGrab();
				if (ConsoleIsOpen()) {
					ConsoleToggle();
					return;
				}
			}
			if (event.key.keysym.sym == SDLK_BACKQUOTE) {
				ConsoleToggle();
				SystemGrab();
				break;
			}
			if (ConsoleIsOpen()) {
				ConsoleInput(event.key.keysym.sym, event.key.keysym.sym);
			}
			else
				InputKeyDown(event.key.keysym.scancode);
			break;
		case SDL_KEYUP:
			if (!ConsoleIsOpen())
				InputKeyUp(event.key.keysym.scancode);
			break;
		case SDL_MOUSEWHEEL:
			if (event.wheel.y < 0)
				InputKeyDown(INPUT_MWHEEL_DOWN);
			if (event.wheel.y > 0)
				InputKeyDown(INPUT_MWHEEL_UP);
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT)
				InputKeyDown(INPUT_LMB);
			if (event.button.button == SDL_BUTTON_RIGHT)
				InputKeyDown(INPUT_RMB);
			break;
		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT)
				InputKeyUp(INPUT_LMB);
			if (event.button.button == SDL_BUTTON_RIGHT)
				InputKeyUp(INPUT_RMB);
			if (event.button.button == SDL_BUTTON_LEFT)
				lmb = false;
			else if (event.button.button == SDL_BUTTON_MIDDLE)
				mmb = false;
			if (!ConsoleIsOpen() && !MenuIsOpen())
				SDL_ShowCursor(false);
			else {
				SDL_ShowCursor(false);
			}
			break;
			//Handle plugging and unplugging of controllers on the fly
		case SDL_CONTROLLERDEVICEREMOVED:
		case SDL_CONTROLLERDEVICEADDED:
			controllers_enumerate();
			break;
		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
				screen_size = GLcoord2(event.window.data1, event.window.data2);

				RenderResize(screen_size.x, screen_size.y);
				MenuResize();
				ini_update();
				SystemGrab();
				break;
			}
		}
	}
}

void SystemGrab()
{
	bool  mlook;

	mlook = true;
	if (ConsoleIsOpen())
		mlook = false;
	if (MenuIsOpen())
		mlook = false;
	InputMouselookSet(mlook);
	if (InputMouselook()) {
		SDL_ShowCursor(false);
		SDL_SetWindowGrab(sdl_window, (SDL_bool)true);
		SDL_SetRelativeMouseMode((SDL_bool)true);
	}
	else {
		SDL_ShowCursor(false);
		SDL_SetWindowGrab(sdl_window, (SDL_bool)false);
		SDL_SetRelativeMouseMode((SDL_bool)false);
	}
}

long SystemTick()
{
	return SDL_GetTicks();
}

GLcoord2 SystemMouse()
{
	return mouse_pos;
}

GLcoord2 SystemSize()
{
	return screen_size;
}

void SystemThread(char* name, int (SDLCALL *fn)(void *), void *data)
{
	Console("SdlThread: Starting thread '%s'", name);
	SDL_CreateThread(fn, name, data);
}

int SystemTime()
{
	return (int)time(NULL);
}

std::string system_env_var(std::string const & key)
{
	char * val = getenv(key.c_str());
	return val == NULL ? std::string("") : std::string(val);
}

string SystemSavePath()
{
	string  slocal;
	string  ssave;
	path    plocal;
	path    psave;

#ifdef _WIN32

	HRESULT result;
	char    char_path[MAX_PATH];

	result = SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, char_path);
	if (result != S_OK) //The above failed. Try to save in our own subdir.
		strcpy(char_path, "./");
	slocal = char_path;
#elif defined(__linux__)
	char * holder = NULL;
	holder = getenv("XDG_CONFIG_HOME");
	if (holder != NULL) {
		slocal = string(holder);
	}
	else {
		holder = getenv("HOME");
		if (holder != NULL) {
			slocal = string(holder) + string("/.config");
		}
		else {
			slocal = string(".") + string("/config");
		}
	}
#elif defined(__APPLE__)
	slocal = string("~/Documents");
#endif

	ssave = slocal + "/Pyrodactyl/Good Robot";

	plocal = path(slocal);
	psave = path(ssave);
	if (!exists(psave))
		create_directories(psave);
	return ssave + "/";
}