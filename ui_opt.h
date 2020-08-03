#pragma once

#include "ui_menu.h"
#include "ui_menuwithtitle.h"
#include "button.h"
#include "ui_resolution.h"
#include "ui_keybind.h"

namespace pyrodactyl
{
	class OptionMenu
	{
		//The state of the menu
		enum MenuState
		{
			STATE_CATEGORY,
			STATE_AUDIO,
			STATE_GRAPHICS,
			STATE_RESOLUTION,
			STATE_KEYBIND,
			STATE_CONTROLLER
		} state;

		//Category menu: audio, graphics, controls, back
		//Audio menu: music, effects, back
		//Graphics menu: fullscreen, resolution, back
		//Controller: back
		MenuWithBgandTitle category, audio, graphics, controller;

		//The resolution value picker
		ResolutionMenu me_resolution;

		//The keyboard remap menu
		KeyBindMenu me_keybind;

		//The image for controller
		ImageData img_controller, img_controller_alt;

	public:
		OptionMenu() { state = STATE_CATEGORY; }
		~OptionMenu() {}

		void Init();

		bool HandleEvents(const bool esc_key_pressed, const bool game_running);
		void Draw();

		void SetUI();
	};
}