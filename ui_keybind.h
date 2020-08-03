#pragma once

#include "common_header.h"
#include "ui_menu.h"
#include "ImageData.h"

namespace pyrodactyl
{
	class KeyBindMenu
	{
		//The menu can be in these 2 states
		enum { STATE_NORMAL, STATE_KEY } state;

		//The background of the menu
		ImageData bg;

		//The title
		Caption title;

		//The reference button we use to populate the menu
		Button ref;

		//inc tells us how much space there is between multiple buttons
		Element inc;

		//The menus for the keyboard options in both categories
		ButtonMenu menu;

		//The back button for this menu
		Button back;

		//The selected button in the current menu
		int choice;

		struct PromptInfo
		{
			int col, col_prev;
			std::string text;

			PromptInfo(){ col = 0; col_prev = 0; }

			void Load(rapidxml::xml_node<char> *node)
			{
				if (NodeValid(node))
				{
					LoadStr(text, "text", node);
					LoadNum(col, "color", node);
				}
			}

			void Swap(Caption &c)
			{
				col_prev = c.col;
				c.text = text;
				c.col = col;
			}
		} prompt;

		void InitMenu();
		void SwapKey(const SDL_Scancode &find);
		void SetCaption();

	public:
		KeyBindMenu(){ Reset(); choice = -1; }
		~KeyBindMenu(){}

		void Reset(){ state = STATE_NORMAL; }

		void Load(rapidxml::xml_node<char> *node);
		bool HandleEvents();

		void Draw();
		void SetUI();
	};
}