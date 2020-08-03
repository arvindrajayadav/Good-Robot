#pragma once

#ifndef _master_h_
#define _master_h_

#define BUILD                     " Release"
#define VER                       "1.1"
#define APPTITLE                  "Good Robot"
#define WINDOWTITLE               APPTITLE " v " VER BUILD

#define FRAMERATE                 60
#define UPDATE_INTERVAL           (1000 / FRAMERATE)

#define WRAP(x,y)                 ((unsigned)x % y)
#define SIGN(x)                   (((x) > 0) ? 1 : ((x) < 0) ? -1 : 0)
#define ABS(x)                    (((x) < 0 ? (-x) : (x)))
#define SMALLEST(x,y)             (ABS(x) < ABS(y) ? 0 : x)
#define SWAP(a,b)                 {int temp = a;a = b; b = temp;}
#define SWAPF(a,b)                {float temp = a;a = b; b = temp;}
#define ARGS(text, args)          { va_list		ap;	text[0] = 0; if (args != NULL)	{ va_start(ap, args); vsprintf(text, args, ap); va_end(ap);}	}
#define INTERPOLATE(a,b,delta)    (a * (1.0f - delta) + b * delta)
#define clamp(n,lower,upper)      (max (min(n,(upper)), (lower)))
#define UNIT(n)                   (CLAMP(n,0,1))

#define DEGREES_TO_RADIANS        .017453292F
#define RADIANS_TO_DEGREES        57.29577951F
#define NEGLIGIBLE                0.000000000001f
#define PI                        (3.1415926535f)
#define GRAVITY                   0.001f //Gain this much speed per frame
#define FOW_SIZE                  128.0f //Fog of war needs to be big enough to cover screen, regardless of camera.

typedef unsigned char uchar;

#pragma warning (disable : 4127) //Conditional expression is constant - e.g. while (1) {}
#pragma warning (disable : 4706) //Assignment within conditional expression - e.g. if (valid = IsValid ()) {}
#pragma warning (disable : 4018) //signed/unsigned mismatch
#pragma warning (disable : 4996) //Microsoft trying to get you to use their platform-specific _stricmp

#define GL_GLEXT_PROTOTYPES
#define GLEW_STATIC

/* Change includes depending on windows / linux */
#ifdef _WIN32
#ifdef _DEBUG
#define WINDEBUG
#endif

#include <windows.h>
#include <Shlobj.h>
#include <math.h>
#include <direct.h>
#include <io.h>
#include <sys/utime.h>
#include <steam/steam_api.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#define _stricmp strcasecmp
#define _strdup strdup
#define _putenv putenv
#define _unlink unlink
#define _isnan isnan
#define stricmp strcasecmp
#include <cmath>
#include <utime.h>
#if defined(__linux__)
#include <sys/io.h>
#endif
#endif

#include <vector>
#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <math.h>
#include <sstream>

#include <SDL.h>
#include "glew/include/GL/glew.h"
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include "IL/il.h"

#ifdef _WIN32
#include "glew/include/GL/wglew.h"
#endif

using namespace std;
using namespace boost::filesystem;

#include "gltypes.h"
#include "vbo.h"
#include "symath.h"
#include "console.h"
#include "string.h"
#include "spritemap.h"

//For debugging, we want these available everywhere, without needing to
//include all the headers.

#if defined(__APPLE__)
#define Console    printf
#else
#define Console    ConsoleLog
#endif
void InterfacePrint(const char* message, ...);
void InterfacePrint(const char* message, ...);

using namespace   std;

typedef uchar   btype;
typedef int     RobotType;

#define LEVELS_XML            "levels.xml"
#define LEVELS_DIR            "core/maps/"
#define LEVELS_EXT            ".tmx"
#define LEVELS_DEFAULT        "empty"
#define GAMEPLAY_FILE         "gameplay.ini"

#define SAVE_FILE_VERSION     4
#define PAGE_SIZE             24
#define PAGE_HALF             (PAGE_SIZE / 2)
#define PAGE_QUARTER          (PAGE_HALF / 2)
#define PAGE_EDGE             (PAGE_SIZE-1)

#define ONE_METER             1.0f
#define ONE_CENTIMETER        (ONE_METER/100.0f)
#define ONE_KILOMETER         (ONE_METER*1000)
#define CM_PER_KM             (ONE_KILOMETER/ONE_CENTIMETER)

//Size of the world grid, in PAGES
#define WORLD_WIDTH           32
#define WORLD_DEPTH           10
#define WORLD_EDGE            (WORLD_WIDTH-1)
#define WORLD_BOTTOM          (WORLD_DEPTH-1)
//Size of the world in units.
#define WORLD_UNITS           (WORLD_WIDTH * PAGE_SIZE)
#define WORLD_UNITS_Y         (WORLD_DEPTH * PAGE_SIZE)
#define LEVEL_INVALID         -1

//The render depth of various elements.
#define DEPTH_PAIN					 0.02f
#define FOE_DEPTH           -0.1f
#define DEPTH_LEVEL          0.15f
#define DEPTH_LEVEL_GLOW     0.153f
#define DEPTH_DOORS          0.14f
#define DEPTH_FX             0.01f
#define DEPTH_FX_GLOW        0.03f
#define DEPTH_VISION         0.1f
#define DEPTH_UNIT_GLOW     -0.05f
#define DEPTH_MACHINES      -0.07f
#define DEPTH_UNITS          0.0f
#define DEPTH_EYES           0.01f
#define DEPTH_PROJECTILES    0.05f
#define DEPTH_BG_NEAR       -1.0f
#define DEPTH_BG_FAR        -4.0f
#define DEPTH_SKY           -6
#define DEPTH_EXPLOSIONS     0.08f
#define DEPTH_MESSAGES       0.151f
#define DEPTH_OVERLAY        1

enum MapColor
{
	COLOR_FOREGROUND,
	COLOR_BACKGROUND1,
	COLOR_BACKGROUND2,
	COLOR_SKY,
	COLOR_LAMP,
	COLOR_LIGHT,
	COLOR_COUNT
};

enum eGameMode
{
	GAME_STORY,
	GAME_SURVIVAL,
	GAME_MODES
};

enum PlayerSkill
{
	SKILL_SHOT_POWER,
	SKILL_SHOT_SPEED,
	SKILL_SPEED,
	SKILL_SHIELDS,
	SKILL_VISION_RADIUS,
	SKILL_COUNT
};

enum ePickupType
{
	ABILITY_MAGNET,
	ABILITY_SCANNER,
	ABILITY_COMPASS,
	//ABILITY_GLIDE, - Removed because its buggy
	ABILITY_TARGET_LASER,
	ABILITY_TYPES,
	ABILITY_INVALID
};

enum PlayerTrivia
{
	TRIVIA_DAMAGE_TAKEN,
	TRIVIA_DAMAGE_DEALT,
	TRIVIA_XP_GATHERED,
	TRIVIA_BULLETS_FIRED,
	TRIVIA_KILLS,
	TRIVIA_DEATHS,
	TRIVIA_CM_TRAVELED,
	TRIVIA_PLAYTIME,
	TRIVIA_MISSILES_DESTROYED,
	TRIVIA_MISSILES_EVADED,
	TRIVIA_COUNT
};

enum PageTraverse
{
	DIRECTION_NONE,
	DIRECTION_RIGHT,
	DIRECTION_LEFT,
	DIRECTION_DOWN,
	DIRECTION_UP,   //Probably not going to be used.
};

struct PageInfo
{
	GLcoord2              grid;
	vector<RobotType >    bots;
	bool                  checkpoint;
	bool                  last;
	bool                  show_stars;
	bool                  opaque;
	int                   connect;
	int                   tileset;
	int                   tileset_overlay;
};

struct Line2D
{
	GLvector2     start;
	GLvector2     end;
};

#endif /* _master_h_ */
