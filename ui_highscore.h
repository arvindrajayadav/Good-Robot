#pragma once

#include "ui_menu.h"
#include "ImageData.h"
#include "ValuePicker.h"
#include "HighScoreData.h"
#include "SteamLeaderboardMenu.h"
#include "LocalLeaderboard.h"

namespace pyrodactyl
{
	class HighScoreMenu
	{
		//The background of the menu
		ImageData bg;

		//The back button
		Button back;

		//The data relevant to the local high score file
		LocalLeaderboard local_leaderboard;

		//The Steam leaderboard data is contained here
		SteamLeaderboardMenu steam_leaderboard;

		//All the types of leaderboards shown
		enum StateType
		{
			//Local record of top 10 runs
			STATE_LOCAL,

			//The Steam leaderboards
			STATE_GLOBAL
		} state;

		//Used to switch between states
		struct StateVal
		{
			int val;
			std::string text;

			void Load(rapidxml::xml_node<char> *node)
			{
				LoadNum(val, "val", node);
				LoadStr(text, "text", node);
			}
		};
		ValuePicker<StateVal> switcher;

	public:
		HighScoreMenu(){ state = STATE_LOCAL; }
		~HighScoreMenu(){}

		void Init();
		void Save();

		void FindLeaderboards() { steam_leaderboard.FindLeaderboards(); }
		void AddScore(const HighScoreData &h);

		bool HandleEvents(const bool esc_key_pressed);
		void Draw();
		void SetUI();
	};
}
