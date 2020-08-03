/*-----------------------------------------------------------------------------

  Game.cpp

  This handles the starting and ending, saving and loading of games.

  Good Robot
  (c) 2014 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "bodyparts.h"
#include "character.h"
#include "console.h"
#include "entity.h"
#include "env.h"
#include "file.h"
#include "game.h"
#include "hud.h"
#include "ini.h"
#include "input.h"
#include "map.h"
#include "menu.h"
#include "noise.h"
#include "player.h"
#include "random.h"
#include "render.h"
#include "robot.h"
#include "system.h"
#include "world.h"

#define MAX_MSG_LEN             1024
#define FILE_SIZE               1024

static int          tick;
static int          frame;
static bool         paused;
static bool         game_running;
static int          autoload;
static int          debug_shopping_id;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void GameCommand(const char* message, ...)
{
	static char     cmd[MAX_MSG_LEN];
	va_list         marker;
	char*           scan;
	vector<string>  words;
	int             word_count;

	//Process the input values into a string.
	va_start(marker, message);
	vsprintf(cmd, message, marker);
	va_end(marker);
	//now break the string apart into an array for easy parsing below.
	scan = strtok(cmd, " ,");
	while (scan) {
		words.push_back(scan);
		scan = strtok(NULL, " ,");
	}
	word_count = words.size();
	if (!word_count)
		return;
	strcpy(cmd, words[0].c_str());

	if (!_stricmp (cmd, "cats")) {
		if (words.size () > 1) {
			if (words[1] == "on")
				EnvValueSetb (ENV_CHEATS, true);
			else if (words[1] == "off")
				EnvValueSetb (ENV_CHEATS, false);
		}
	}
	if (!_stricmp (cmd, "bots") || !_stricmp (cmd, "robots") || !_stricmp (cmd, "rollcall")) {
		Console ("EnvInit: Robot roll call:");
		Console ("%s", EnvRobotRollCall ());
	}
	if (!_stricmp (cmd, "hud")) {
		if (words.size () > 1) {
			if (words[1] == "on")
				HudVisible (true);
			else if (words[1] == "off")
				HudVisible (false);
		} else
			HudToggleVisible ();
	}
	if (EnvHandleCommand (words))
		return;
	if (!EnvValueb (ENV_CHEATS))
		return;
	if (!_stricmp(cmd, "reload"))
		EnvReloadData();
	if (!_stricmp(cmd, "compile"))
		RenderCompile();
	if (!_stricmp(cmd, "die")) {
		EnvValueSetb(ENV_NODIE, false);
		PlayerDamage(9909);
	}
	if (!_stricmp(cmd, "track")) {
		if (words.size() > 1)
			AudioPlaySong(words[1].c_str());
	}
	if (!_stricmp(cmd, "crash")) {
		GLvector2*      v = NULL;
		v->Normalize();
	}
	if (!_stricmp(cmd, "mode")) {
		eGameMode gm;
		if (words.size() > 1) {
			gm = (eGameMode)atoi(words[1].c_str());
			Player()->GameModeSet(gm);
		}
		else
			gm = Player()->GameMode();
		Console("Difficulty set to %s.", GameModeString(gm).c_str());
	}
	if (!_stricmp(cmd, "zone")) {
		if (words.size() > 1) {
			int zone = atoi(words[1].c_str());
			WorldZoneTransition(zone);
			Console("Moving to zone #%d", zone);
		}
	}
	if (!_stricmp(cmd, "motif")) {
		if (words.size() > 1) {
			int motif = atoi(words[1].c_str());
			WorldSetMotif(motif);
			Console("Setting to motif #%d", motif);
		}
	}
	if (!_stricmp(cmd, "shopping")) {
		debug_shopping_id++;
		Console("Refreshing shop inventory.");
	}
	if (!_stricmp(cmd, "go")) {
		Robot b;
		for (int c = 0; c < 300; c++) {
			b.Init(PlayerPosition() + GLvector2(0.5f, 0), "Shooter1");
			EntityRobotAdd(b);
		}
	}
	if (!_stricmp(cmd, "map")) {
		if (words.size() < 2) {
			Console("Usage: map <number> <zone>");
		}
		else if (words.size() == 3)
			WorldMapSet(atoi(words[1].c_str()), atoi(words[2].c_str()));
		else
			WorldMapSet(atoi(words[1].c_str()), 0);
	}
	if (!_stricmp(cmd, "new")) {
		eGameMode   gm = GAME_STORY;
		int         character = 0;

		if (words.size() > 1)
			character = EnvCharacterIndexFromName(words[1]);
		if (words.size() > 2)
			gm = GameMode(words[2]);
		GameNew(character, gm);
	}
	if (!_stricmp(cmd, "spawn")) {
		if (words.size() < 2) {
			Console("Usage: spawn <robot>");
			Console(EnvRobotRollCall());
			return;
		}
		RobotType id = EnvRobotIndexFromName(words[1].c_str());
		if (id != ROBOT_INVALID) {
			Robot b;
			int   count;

			count = 1;
			if (word_count > 2)
				count = atoi(words[2].c_str());
			for (int c = 0; c < count; c++) {
				b.Init(PlayerPosition() + GLvector2(1.5f, 0), id);
				EntityRobotAdd(b);
			}
		}
	}
	if (!_stricmp(cmd, "gimmie")) {
		PlayerStats* p = Player();
		p->MoneyGive(100000);
	}
	if (!_stricmp (cmd, "win")) {
		WorldFinalBossKill();
	}
}

int GameShoppingId() { return debug_shopping_id; }

string GameModeString(eGameMode d)
{
	switch (d) {
	case GAME_STORY: return "Story Mode";
	case GAME_SURVIVAL: return "Survival Mode";
	default: return "UNKNOWN";
	}
}

eGameMode GameMode(string name)
{
	name = StringToLower(name);
	if (name.compare("survival"))
		return GAME_SURVIVAL;
	return GAME_STORY;
}

void GameNew(int character, eGameMode gm)
{
	PlayerStats*  ps;
	string        name;

	name = EnvCharacter(character)._name;
	Console("Starting new game as %s on %s mode.", name.c_str(), GameModeString(gm).c_str());
	ps = Player();
	ps->Init(character, gm);
	WorldMapSet(0, 0);
	MenuReset();
	game_running = true;
}

void GameNew(eGameMode gm)
{
	GameNew(0, gm);
}

void GameQuit()
{
	game_running = false;
}

string GameSaveFile(eGameMode gm)
{
	if (gm == GAME_STORY)
		return SystemSavePath() + "story.sav";
	if (gm == GAME_SURVIVAL)
		return SystemSavePath() + "survival.sav";

	return SystemSavePath() + "unknown.sav";
}

void GameEnd()
{
	FileDelete(GameSaveFile(Player()->GameMode()));
}

void GameSave()
{
	Player()->Save(GameSaveFile(Player()->GameMode()));
}

void GameLoad(eGameMode gm)
{
	PlayerStats*  ps;

	ps = Player();
	ps->Init(0, gm);
	if (!ps->Load(GameSaveFile(gm))) {
		game_running = false;
		return;
	}
	NoiseSeed(ps->Id());
	WorldMapSet(ps->CheckpointGet().map, ps->CheckpointGet().zone);
	game_running = true;
}

void GameTerm()
{
	if (game_running) {
		iniFile   f;

		GameSave();

		f.Open(SystemConfigFile());
		f.IntSet("Settings", "LastSave", Player()->Id());
	}
}

int GameTick()
{
	return tick;
}

int GameFrame ()
{
	return frame;
}

void GameUpdate()
{
	bool        pause_music;

	if (autoload) {
		GameLoad(GAME_STORY);
		autoload = 0;
	}
	pause_music = false;
	if (ConsoleIsOpen() || (MenuOpenGet() == MENU_MAIN && GameRunning()))
		pause_music = true;
	AudioPauseMusic(pause_music);
	//need_pause = false;
	if (game_running) {
		if (ConsoleIsOpen() || MenuIsOpen()) {
			paused = true;
			return;
		}
	}
	paused = false;
	//Below only happens when game is NOT paused.
	tick += UPDATE_INTERVAL;
	frame++;
}

bool GameActive()
{
	return GameRunning() && !GamePaused();
}

bool GameRunning()
{
	return game_running;
}

bool GamePaused()
{
	return paused;
}

void GameRender()
{
	WorldRender();
	HudRender();
}

void GameInit()
{
#ifdef _DEBUG
	iniFile   f;

	f.Open(SystemConfigFile());
	autoload = f.IntGet("Settings", "LastSave");
	//autoload = 0;
#endif
}