/*-----------------------------------------------------------------------------

  Main.cpp

  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "avatar.h"
#include "audio.h"
#include "camera.h"
#include "drop.h"
#include "env.h"
#include "file.h"
#include "font.h"
#include "game.h"
#include "hud.h"
#include "ini.h"
#include "interface.h"
#include "lootpool.h"
#include "main.h"
#include "menu.h"
#include "message.h"
#include "noise.h"
#include "particle.h"
#include "player.h"
#include "random.h"
#include "render.h"
#include "sprite.h"
#include "system.h"
#include "texture.h"
#include "trivia.h"
#include "visible.h"
#include "world.h"
#include "steam_data.h"
#include "TMXMap.h"
#include "timer.h"

#ifdef CMAKE_BUILD
#include "steam_api.h"
#endif

#ifdef _DEBUG
#define RANDOM_SEED   8
#else
#define RANDOM_SEED   SystemTick ()
#endif

#pragma comment (lib, "opengl32.lib")                                       //OpenGL
#pragma comment (lib, "glu32.lib")                                          //OpenGL
#pragma comment (lib, ".\\sdl2\\lib\\x86\\sdl2.lib")                               //Good 'ol SDL.
#pragma comment (lib, ".\\openal\\libs\\win32\\openal32.lib")               //For Audio
#pragma comment (lib, ".\\openal\\libs\\win32\\alut.lib")                   //For Audio
#pragma comment (lib, ".\\openil\\lib\\DevIL.lib")                          //For loading images
#pragma comment (lib, ".\\freetype\\lib\\freetype.lib")                     //For fonts sake
#pragma comment (lib, ".\\steamworks\\redistributable_bin\\steam_api.lib") //Steam

static bool             quit;
static int              time_counter;
static int              free_time;
static int              next_second;
static int              profile_update;
static int              profile_test;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void init()
{ 
	SystemInit();
	ilInit();         //Must come after system.
	SpriteMapInit();  //Must come after iL (Image library.)
	AudioInit();
	EnvInit();
	LootpoolInit();  //Must come after Env
	DropInit();      //Must come after lootpool
	RenderInit();     //Must come after system.
	InterfaceInit();
	RandomInit(RANDOM_SEED);
	FontInit();
	GameInit();
	NoiseInit();
	WorldInit();
	ConsoleInit();
	MessageInit();
	MenuInit();
	VisibleInit();
	PlayerInit();
	ParticleInit();
	HudInit();
	SteamAPI_Init();
	SteamUserInit();
}

static void run()
{
	int           next_frame;
	int           leftover;

	//Use a timer for fps
	pyrodactyl::Timer fps;

	next_frame = 0;
	while (!quit)	{
		//Start the frame timer
		fps.Start();
		AudioUpdate();
		GameUpdate();
		MenuUpdate();
		PlayerUpdate();
		HudUpdate();
		CameraUpdate();
		TriviaUpdate();
		VisibleUpdate();
		ParticleUpdate();
		WorldUpdate();
		SystemUpdate();
		TextureUpdate();
		FontUpdate();
		ConsoleUpdate();
		Render();
		SteamAPI_RunCallbacks();
		leftover = next_frame - SystemTick();
		if (leftover > 0)
			time_counter += leftover;

		SystemSwapBuffers();

		if (!EnvValueb(ENV_FPSUNCAP) && fps.Ticks() < UPDATE_INTERVAL)
			SDL_Delay(UPDATE_INTERVAL - fps.Ticks());
	}
}

static void term()
{
	GameTerm();
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

int MainFreeTime()
{
	return free_time;
}

char* MainTitle()
{
	return APPTITLE;
}

void MainQuit()
{
	quit = true;
}

bool MainIsQuit()
{
	return quit;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

#ifdef _WIN32
int PASCAL WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else //linux
int main(int argc, char** argv)
#endif
{
	init();
	run();
	term();
	SteamAPI_Shutdown();
	return 0;
}