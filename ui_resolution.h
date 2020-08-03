#pragma once

#include "common_header.h"
#include "system.h"
#include "ui_menu.h"
#include "ImageData.h"
#include "timer.h"

namespace pyrodactyl
{
	class ResolutionMenu
	{
		enum State
		{
			//Normal shows all the resolution buttons
			STATE_NORMAL,

			//We ask players if they want to keep the new resolution for X seconds
			STATE_CONFIRM
		} state;

		//The backgrounds for different states
		ImageData bg_res, bg_confirm;

		//The title for the normal state
		Caption title;

		//Tells us the current resolution in normal state
		Caption res_info;
		std::string res_prefix;

		//The reference button for resolution
		Button ref;

		//How much the button is incremented by
		Element inc;

		//The menu containing the buttons representing dim
		ButtonMenu me_res;

		//We ask players if they want to keep the new resolution for X seconds
		//Button 0 is accept, button 1 is cancel
		ButtonMenu me_confirm;

		//The back button for resolution menu
		Button back;

		//The current resolution index and the previous one (used for switching back to the old one)
		int cur_index, prev_index;

		//The countdown until the timer resets
		TextData countdown;
		std::string prefix;
		Timer timer;

		//The message shown when confirm menu is on display
		HoverInfo msg;

		void ResolutionSet(int val);

	public:
		ResolutionMenu(void){ state = STATE_NORMAL; cur_index = 0; prev_index = cur_index; }
		~ResolutionMenu(void){}

		void Load(rapidxml::xml_node<char> *node);
		void Draw();

		//Return true if we need to exit to category menu
		bool HandleEvents(const bool game_running);

		void SetUI();
	};
}
