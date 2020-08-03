#pragma once

#include "ScoreButton.h"
#include "ui_menu.h"

namespace pyrodactyl
{
	//The data relevant to the local high score file
	class LocalLeaderboard
	{
		//The data
		std::vector<HighScoreData> score;

		//The maximum number of entries we store to file
		int max_entries;

		//The button that tells you what each column means (drawn above the menu)
		ScoreButton legend;

		//The menu we use to display the scores
		Menu<ScoreButton> menu;

		//The template/reference button for the menu
		ScoreButton ref;

		//The value by which menu buttons are incremented each time
		Element inc;

	public:
		LocalLeaderboard(){ max_entries = 10; }

		void AddScore(const HighScoreData &h);

		void HandleEvents();
		void Draw();
		void SetUI();

		void Load(rapidxml::xml_node<char> *node);
		void Save();
	};
}