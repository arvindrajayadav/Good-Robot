/*-----------------------------------------------------------------------------

  Menu.cpp

  Interface manager.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "game.h"
#include "input.h"
#include "main.h"
#include "menu.h"
#include "message.h"
#include "render.h"
#include "system.h"
#include "hud.h"

//For the "add to high score list"
#include "player.h"

//Pyrodactyl stuff
#include "ui_mainmenu.h"
#include "ui_upgrade.h"
#include "ui_gameover.h"
#include "ui_store.h"
#include "ui_hatshop.h"
#include "steam_data.h"

pyrodactyl::MainMenu     menu_main;
pyrodactyl::UpgradeMenu  menu_upgrade;
pyrodactyl::GameOverMenu menu_gameover, menu_win;
pyrodactyl::StoreMenu    menu_store;
pyrodactyl::HatShopMenu  menu_hat;

static int               current_menu;
/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void PlayerStats::SaveHighScore()
{
	using namespace pyrodactyl;
	HighScoreData h;

	h.valid = true;
	h.name = SteamDisplayName();
	h.score = _score_points;
	h.kills = _trivia[TRIVIA_KILLS];
	h.money_earned = _trivia[TRIVIA_XP_GATHERED];
	h.upgrade_count = _upgrade_count;
	h.warranty_count = _warranty_count;
	h.playtime = _trivia[TRIVIA_PLAYTIME]; 	//In milliseconds

	menu_main.AddScore(h);
}

void MenuLoadLeaderboards()
{
	menu_main.FindLeaderboards();
}

void MenuInit()
{
	//Pyrodactyl stuff
	{
		using namespace pyrodactyl;
		gImageManager.Init();
		gTextManager.Init();
		gInput.Init();
		menu_main.Init();
		menu_upgrade.Init();
		menu_gameover.Init("core/data/ui_gameover.xml");
		menu_win.Init("core/data/ui_win.xml");
		menu_store.Init();
		menu_hat.Init();
	}
	current_menu = MENU_NONE;
}

bool MenuIsOpen()
{
	return current_menu != MENU_NONE;
}

int MenuOpenGet()
{
	return current_menu;
}

void MenuReset()
{
	menu_store.Reset();
}

//IF YOU WANT TO RESIZE OR REPOSITION WINDOWS, THIS IS THE FUNCTION
void MenuOpen(int menu_num)
{
	current_menu = menu_num;
	InputClearState();
	MenuResize();

	//Reset the "accept" key presses, otherwise you end up buying the first
	//thing in the store because it carries over
	using namespace pyrodactyl;
	gInput.Pressed(CONTROL_ACTIVATE);

	//Play the appropriate song for the menus
	switch (menu_num)
	{
	case MENU_MAIN:
		if (!GameRunning())
			AudioPlaySong(menu_main.music_track.c_str(), false);
		break;
	case MENU_GAMEOVER:
		AudioPlaySong(menu_gameover.music_track.c_str(), false);
		break;
	case MENU_WIN:
		AudioPlaySong(menu_win.music_track.c_str(), false);
		break;
	default: break;
	}
}

//Used when user resizes window
void MenuResize()
{
	HudSetUI();
	switch (current_menu)
	{
	case MENU_MAIN: menu_main.Update(); menu_main.SetUI(); break;
	case MENU_UPGRADE: menu_upgrade.PopulateMenu(); menu_upgrade.SetUI(); break;
	case MENU_STORE: menu_store.Randomize(); menu_store.SetUI(); break;
	case MENU_GAMEOVER: menu_gameover.Update(); menu_gameover.SetUI(); break;
	case MENU_WIN: menu_win.Update(); menu_win.SetUI(); break;
	case MENU_HAT: menu_hat.Randomize(); menu_hat.SetUI(); break;
	default: break;
	}
	SystemGrab();
}

void MenuUpdate()
{
	using namespace pyrodactyl;
	GLcoord2  mouse;

	//Escape is the keyboard key for both pause and back, which requires special checking
	bool esc_key_pressed = InputKeyPressed(gInput.iv[CONTROL_PAUSE].key_val);

	if (GameRunning())
	{
		//We're inside the game
		if (gInput.Pressed(CONTROL_PAUSE) || esc_key_pressed)
		{
			if (current_menu == MENU_NONE)
			{
				if (Player()->Dead() && !Player()->Warranty ())
				{
					GameEnd();
					MenuOpen(MENU_GAMEOVER);
				}
				else
				{
					MenuOpen(MENU_MAIN);

					//We exit this function because otherwise esc_key_pressed
					//will resume the game again
					return;
				}
			}
			else if (current_menu == MENU_MAIN && menu_main.OutsideMenu())
			{
				//Go back to the game if we're not inside any sub menu in the pause menu
				MenuOpen(MENU_NONE);
			}
		}
	}
	else if (current_menu == MENU_NONE)
	{
		//If no game is running, then the main menu should be open,
		//otherwise we have a blank screen with nothing to do.
		MenuOpen(MENU_MAIN);
	}

	if (!MenuIsOpen())
		return;

	//This is the main menu
	if (current_menu == MENU_MAIN)
	{
		MainMenuSignal result = menu_main.HandleEvents(esc_key_pressed, GameRunning());
		if (result != SIG_NONE)
		{
			switch (result)
			{
			case SIG_LOAD:
				//Continue existing game from save file
				//If no existing save game exists, hide this button (Arvind will handle the hide button part)
				GameLoad(GAME_STORY);
				MenuOpen(MENU_NONE);
				break;
			case SIG_NEW:
				//Open new game menu, where player chooses game mode and modifiers
				GameNew(GAME_STORY);
				MenuOpen(MENU_NONE);
				break;
			case SIG_CHALLENGE:
				//Start daily challenge mode - instant start, no mode select and no save file created
				MenuOpen(MENU_NONE);
				break;
			case SIG_RESUME:
				//Only sent by the pause menu
				MenuOpen(MENU_NONE);
				break;
			case SIG_SAVE_QUIT_TO_MAINMENU:
				//Save the game, then exit to main menu (this signal is only sent if the player is in-game)
				//Disabled the save here to prevent level cheesing for infinite money/score
				//GameSave();
				GameQuit();
				MenuOpen(MENU_NONE);
				break;
			case SIG_EXIT_TO_OS:
				//Exit the game (this signal is only sent if the player is in the main menu)
				MainQuit();
				break;
			default: break;
			}
		}
	}
	else if (GameRunning())
	{
		switch (current_menu)
		{
		case MENU_UPGRADE:
			if (menu_upgrade.HandleEvents(esc_key_pressed))
				MenuOpen(MENU_NONE);
			break;
		case MENU_STORE:
			if (menu_store.HandleEvents(esc_key_pressed))
				MenuOpen(MENU_NONE);
			break;
		case MENU_GAMEOVER:
		{
			int choice = menu_gameover.HandleEvents();

			//Pressing the escape key is same as "go back to main menu"
			if (esc_key_pressed)
				choice = 1;

			switch (choice)
			{
			case 0: GameNew(GAME_STORY); MenuOpen(MENU_NONE); break;
			case 1: GameQuit(); MenuOpen(MENU_NONE); break;
			default: break;
			}
		}
		break;
		case MENU_WIN:
		{
			int choice = menu_win.HandleEvents();

			//Pressing the escape key is same as "go back to main menu"
			if (esc_key_pressed)
				choice = 1;

			switch (choice)
			{
			case 0: GameNew(GAME_STORY); MenuOpen(MENU_NONE); break;
			case 1: GameQuit(); MenuOpen(MENU_NONE); break;
			default: break;
			}
		}
		break;
		case MENU_HAT:
			if (menu_hat.HandleEvents(esc_key_pressed))
				MenuOpen(MENU_NONE);
			break;
		default:break;
		}
	}
}

void MenuRender2D()
{
	if (current_menu == MENU_NONE)
		return;

	if (current_menu == MENU_MAIN)
		menu_main.Draw(GameRunning());
	else if (GameRunning())
	{
		switch (current_menu)
		{
		case MENU_UPGRADE:
			menu_upgrade.Draw();
			break;
		case MENU_STORE:
			menu_store.Draw();
			break;
		case MENU_GAMEOVER:
			menu_gameover.Draw();
			break;
		case MENU_WIN:
			menu_win.Draw();
			break;
		case MENU_HAT:
			menu_hat.Draw();
			break;
		default:break;
		}
	}
}

void MouseRender()
{
	if (!InputMouselook())
	{
		GLcoord2 pos = InputMousePosition();
		menu_main.mouse_img.Draw(pos.x, pos.y, InputKeyState(INPUT_LMB));
	}
}