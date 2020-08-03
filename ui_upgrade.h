#pragma once

#include "UpgradeButton.h"
#include "ImageData.h"
#include "player.h"
#include "ui_news.h"

namespace pyrodactyl
{
	class UpgradeMenu
	{
		//The background image
		ImageData bg;

		//The title
		Caption title;

		//The template button for all abilities
		UpgradeButton t_bu;

		//By how much is the button position incremented each time
		Element inc;

		//All the different skills the player can upgrade
		Menu<UpgradeButton> upgrade;

		//The close button
		Button quit;

		//We need to repopulate the skill buttons
		bool refresh;

		//Button image _b = normal, _h when money is increased, _s when money is decreased
		//Caption = money value, desc = the text "money"
		Button money;

		//The money symbol
		std::string money_sym;

		//The tooltip which displays the description for each upgrade
		HoverInfo tooltip;

		//News is displayed using this
		NewsDisplay news;

		//Keep track of where the last store we opened was
		//if it is the same location as the current one, don't randomize weapons
		int last_world_loc_id;

		//Data for the ability power-ups offered to the player
		struct AbilityData
		{
			//The selected ability for the menu
			int select;

			struct ButtonData
			{
				std::string caption, tooltip;
				int cost;

				ButtonData(){ cost = 1; }
			};

			ButtonData data[ABILITY_TYPES];

			//Text shown on button when player owns the ability
			ButtonData owned;

			AbilityData(){ select = 0; }
		} ability;

	public:
		UpgradeMenu(){ refresh = true; last_world_loc_id = -1; }
		~UpgradeMenu(){}

		void Init();
		bool HandleEvents(const bool esc_key_pressed);
		void Draw();

		void PopulateMenu();
		void SetUI();
	};
}