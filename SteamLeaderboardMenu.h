#pragma once

#include "SteamLeaderboard.h"
#include "SteamLeaderboardData.h"
#include "SteamLeaderboardButton.h"
#include "ValuePicker.h"
#include "ui_menu.h"

namespace pyrodactyl
{
	//The data relevant to the local high score file
	class SteamLeaderboardMenu
	{
		//The button that tells you what each column means (drawn above the menu)
		SteamLeaderboardButton legend;

		//The menu we use to display the scores
		Menu<SteamLeaderboardButton> menu;

		//The template/reference button for the menu
		SteamLeaderboardButton ref;

		//The value by which menu buttons are incremented each time
		Element inc;

		//A general notice when data is loading or an error happens
		Caption notice;

		enum LeaderboardType
		{
			//Score leaderboard
			STATE_SCORE,

			//Kills leaderboard
			STATE_KILLS
		} state;

		enum SortingType
		{
			//The user and their friends only
			SORT_FRIENDS,

			//The user's rank displayed in the global leaderboards
			SORT_GLOBAL_AROUND_USER,

			//The top global rankings
			SORT_GLOBAL
		} sort;

		//Used to switch between states and sorting types
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
		ValuePicker<StateVal> switcher_state, switcher_sort;

		//The steam leaderboard
		SteamLeaderboard leaderboard;

		//The data downloaded from Steam
		SteamLeaderboardData score_data;

		//The type of request sent to Steam
		ELeaderboardDataRequest request_type;

		void HideMenu()
		{
			for (int i = 0; i < score_data.k_nMaxLeaderboardEntries; ++i)
				menu.element.at(i).Visible(false);
		}

		//The first run, used to download the initial set of scores
		bool first_run;

		//Need to reconstruct menu
		bool rebuild;

		//The offset at which scores are downloaded (i.e. 10 entries after X, or 20 entries before X)
		int offset;

		//The buttons to change the offset by score_data.k_nMaxLeaderboardEntries
		Button inc_offset, dec_offset;

		//Button to reset the offset
		Button reset_offset;

	public:
		SteamLeaderboardMenu(){ state = STATE_SCORE; rebuild = false; first_run = true; offset = 0; }

		void Load(rapidxml::xml_node<char> *node);

		void FindLeaderboards() { leaderboard.FindLeaderboards(); }
		void AddScore(int score, int kills) { leaderboard.AddScore(score, kills); }

		void DownloadScores();
		void Rebuild();

		void HandleEvents();
		void Draw();
		void SetUI();
	};
}