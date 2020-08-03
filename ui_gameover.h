#pragma once

#include "ui_menu.h"
#include "ImageData.h"

namespace pyrodactyl
{
	class GameOverMenu
	{
		//The background
		ImageData menu_bg, img_bg;

		//Do we need to draw the image bg
		bool draw_img;

		//The title and the random quote layout
		Caption title, text;

		struct Quote
		{
			std::string title, text;
		};

		struct QuoteWithNumbers
		{
			std::string title, text;
			int n1, n2;
		};

		//Choose one out of these quotes to display
		std::vector<Quote> quote;

		//The menu which has 2 buttons - retry and quit
		ButtonMenu me_act;

		//The reference button for resolution
		Button ref;

		//How much the button is incremented by
		Element inc;

		//The menu containing the buttons used for drawing trivia
		ButtonMenu me_trivia;

		//What are the two funny stats called?
		std::vector<QuoteWithNumbers> fun_stat;

	public:

		//The music track used for the menu
		std::string music_track;

		GameOverMenu(){ draw_img = false; }
		~GameOverMenu(){}

		void Init(const char* filename);
		void Update();

		int HandleEvents();
		void Draw();

		void SetUI();
	};
}