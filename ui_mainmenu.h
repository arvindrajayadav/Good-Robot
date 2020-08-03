#pragma once

#include "ui_menuwithtitle.h"
#include "ui_menu.h"
#include "ui_opt.h"
#include "ui_highscore.h"
#include "ui_credits.h"

namespace pyrodactyl
{
	enum MainMenuSignal { SIG_NONE, SIG_LOAD, SIG_NEW, SIG_CHALLENGE, SIG_RESUME, SIG_SAVE_QUIT_TO_MAINMENU, SIG_EXIT_TO_OS };

	class MainMenu
	{
		enum MenuState
		{
			STATE_NORMAL,
			STATE_DIFFICULTY,
			STATE_OPTIONS,
			STATE_HIGHSCORE,
			STATE_CREDITS,
			STATE_OVERWRITE
		} state;

		//The background and logo
		ImageData bg_main, logo;

		//The main menu
		ButtonMenu me_main;

		//The difficulty menu
		MenuWithBgandTitle me_diff;

		//Options and help menu
		OptionMenu me_opt;

		//The color of the background for pause menu
		int col_pause;

		//The pause menu background
		ImageData bg_pause;

		//The pause menu
		ButtonMenu me_pause;

		//The high score menu
		HighScoreMenu me_highscore;

		//The credits
		CreditScreen me_credits;

		//The overwrite menu
		struct
		{
			ImageData bg;
			Caption caption_0, caption_1;
			ButtonMenu menu;

			void Load(rapidxml::xml_node<char> *node)
			{
				if (NodeValid("bg", node))
					bg.Load(node->first_node("bg"));

				if (NodeValid("caption_0", node))
					caption_0.Load(node->first_node("caption_0"));

				if (NodeValid("caption_1", node))
					caption_1.Load(node->first_node("caption_1"));

				if (NodeValid("menu", node))
					menu.Load(node->first_node("menu"));
			}

			void Draw() { bg.Draw(); caption_0.Draw(); caption_1.Draw(); menu.Draw(); }
			void SetUI() { bg.SetUI(); caption_0.SetUI(); caption_1.Draw(); menu.SetUI(); }
		} overwrite;

	public:
		MainMenu() { state = STATE_NORMAL; col_pause = 0; }
		~MainMenu() {}

		struct MouseImg : public Element
		{
			ImageKey normal, click;

			MouseImg() { normal = 0; click = 0; }

			void Draw(const int x, const int y, const bool &mouse_down)
			{
				if (mouse_down)
					gImageManager.Draw(x, y, w, h, click);
				else
					gImageManager.Draw(x, y, w, h, normal);
			}
		} mouse_img;

		//The music track used for the menu
		std::string music_track;

		void Init();
		void Update();

		MainMenuSignal HandleEvents(const bool esc_key_pressed, const bool game_running);
		void Draw(const bool game_running);

		void SetUI();

		void AddScore(const HighScoreData &h) { me_highscore.AddScore(h); }

		//Are we inside any sub menus or not
		bool OutsideMenu() { return state == STATE_NORMAL; }

		//Load leaderboards from Steam
		void FindLeaderboards() { me_highscore.FindLeaderboards(); }
	};
}