#include "master.h"
#include "ui_mainmenu.h"
#include "file.h"
#include "game.h"
#include "env.h"

using namespace pyrodactyl;

void MainMenu::Init()
{
	XMLDoc layout_doc("core/data/ui_layout.xml");
	if (layout_doc.ready())
	{
		rapidxml::xml_node<char> *node = layout_doc.Doc()->first_node("ui");
		if (NodeValid(node))
		{
			if (NodeValid("bg", node))
				bg_main.Load(node->first_node("bg"));

			if (NodeValid("logo", node))
				logo.Load(node->first_node("logo"));

			if (NodeValid("audio", node))
				LoadStr(music_track, "track", node->first_node("audio"));

			if (NodeValid("mouse", node))
			{
				rapidxml::xml_node<char> *mounode = node->first_node("mouse");
				LoadNum(mouse_img.normal, "img", mounode);
				LoadNum(mouse_img.click, "img_h", mounode);
				mouse_img.Load(mounode);
			}

			if (NodeValid("main_menu", node))
				me_main.Load(node->first_node("main_menu"));

			if (NodeValid("diff_menu", node))
				me_diff.Load(node->first_node("diff_menu"));

			if (NodeValid("pause_menu", node))
			{
				rapidxml::xml_node<char> *panode = node->first_node("pause_menu");

				if (NodeValid("menu", panode))
					me_pause.Load(panode->first_node("menu"));

				if (NodeValid("bg", panode))
					bg_pause.Load(panode->first_node("bg"));

				LoadNum(col_pause, "col", panode);
			}

			if (NodeValid("overwrite", node))
				overwrite.Load(node->first_node("overwrite"));
		}
	}

	me_highscore.Init();
	me_opt.Init();
	me_credits.Init();
}

void MainMenu::Update()
{
	me_main.element.at(0).Visible(FileExists(GameSaveFile(GAME_STORY)));
	me_main.AssignPaths();
}

void MainMenu::Draw(const bool game_running)
{
	if (game_running)
	{
		//Apply color to dim the background
		GLvector2 size = RenderViewportSize();
		GLrgba col = gTextManager.GetColor(col_pause);

		glBindTexture(GL_TEXTURE_2D, 0);
		glBegin(GL_QUADS);
		glColor4f(col.red, col.green, col.blue, col.alpha);
		glVertex3f(0, 0, 0);
		glVertex3f(size.x, 0, 0);
		glVertex3f(size.x, size.y, 0);
		glVertex3f(0, size.y, 0);
		glEnd();

		if (state == STATE_NORMAL)
			bg_pause.Draw();
	}
	else
		bg_main.Draw();

	switch (state)
	{
	case STATE_NORMAL:
		if (game_running)
			me_pause.Draw();
		else
		{
			logo.Draw();
			me_main.Draw();
		}
		break;
	case STATE_DIFFICULTY:
		me_diff.Draw();
		break;
	case STATE_OPTIONS:
		me_opt.Draw();
		break;
	case STATE_HIGHSCORE:
		me_highscore.Draw();
		break;
	case STATE_CREDITS:
		me_credits.Draw();
		break;
	case STATE_OVERWRITE:
		overwrite.Draw();
		break;
	default:break;
	}
}

MainMenuSignal MainMenu::HandleEvents(const bool esc_key_pressed, const bool game_running)
{
	switch (state)
	{
	case STATE_NORMAL:
	{
		if (game_running)
		{
			if (esc_key_pressed)
				return SIG_RESUME;

			int result = me_pause.HandleEvents();
			switch (result)
			{
			case 0: return SIG_RESUME;
			case 1: state = STATE_HIGHSCORE; break;
			case 2: state = STATE_OPTIONS; break;
			case 3: return SIG_SAVE_QUIT_TO_MAINMENU;
			default:break;
			}
		}
		else
		{
			int result = me_main.HandleEvents();
			switch (result)
			{
			case 0: return SIG_LOAD;
			case 1:
				if (FileExists(GameSaveFile(GAME_STORY)))
					state = STATE_OVERWRITE;
				else
					return SIG_NEW;
				break;
			case 2: state = STATE_HIGHSCORE; break;
			case 3: state = STATE_OPTIONS; break;
			case 4: state = STATE_CREDITS; break;
			case 5: return SIG_EXIT_TO_OS;
			default:break;
			}
		}
	}
	break;
	case STATE_DIFFICULTY:
	{
		if (esc_key_pressed)
		{
			state = STATE_NORMAL;
			break;
		}

		int choice = me_diff.menu.HandleEvents();
		if (choice > -1)
			switch (choice)
			{
			case 0:
				EnvSetDifficulty(false);
				state = STATE_NORMAL;
				return SIG_NEW;

			case 1:
				EnvSetDifficulty(true);
				state = STATE_NORMAL;
				return SIG_NEW;

			default: state = STATE_NORMAL; break;
			}
	}
	break;
	case STATE_OPTIONS:
		if (me_opt.HandleEvents(esc_key_pressed, game_running))
			state = STATE_NORMAL;
		break;
	case STATE_HIGHSCORE:
		if (me_highscore.HandleEvents(esc_key_pressed))
			state = STATE_NORMAL;
		break;
	case STATE_CREDITS:
		if (me_credits.HandleEvents(esc_key_pressed))
			state = STATE_NORMAL;
		break;
	case STATE_OVERWRITE:
	{
		//Should we overwrite the save file
		//Used when players have an existing save and they start a new game
		int result = overwrite.menu.HandleEvents();
		if (result == 0)
			state = STATE_DIFFICULTY;
		else if (result == 1)
			state = STATE_NORMAL;
	}
	break;
	default:break;
	}

	return SIG_NONE;
}

void MainMenu::SetUI()
{
	bg_main.SetUI();
	bg_pause.SetUI();
	logo.SetUI();

	me_main.SetUI();
	me_diff.SetUI();
	me_opt.SetUI();
	me_pause.SetUI();
	me_highscore.SetUI();
	me_credits.SetUI();

	overwrite.SetUI();
	mouse_img.SetUI();
}