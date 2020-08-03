#pragma once

#include "ui_menu.h"
#include "StoreButton.h"
#include "player.h"
#include "ui_tutorial.h"

#define STORE_HAT_SLOTS 6

namespace pyrodactyl
{
	class HatShopMenu
	{
		//The background
		ImageData bg;

		//The title of the store
		Caption title;

		//The template button for the store buttons
		StoreButton t_bu;

		//By how much is the button position incremented each time
		Element inc;

		//The different objects you can buy
		Menu<StoreButton> menu;

		//Button image _b = normal, _h when money is increased, _s when money is decreased
		//Caption = money value, desc = the text "money"
		Button money;

		//The money symbol
		std::string money_sym;

		//The button which sends you back to main menu
		Button quit;

		//We need to refresh the costs
		bool refresh;

		//Keep track of where the last store we opened was
		//if it is the same location as the current one, don't randomize weapons
		int last_world_loc_id;

		//The tooltip for the description of stuff you can buy at the store
		HoverInfo tooltip;

		//Display several tooltips to the player
		TutorialDisplay tutorial;

		//A list of hats and their details
		struct HatInfo
		{
			//The id of set
			int id;

			//Name and description
			std::string name, desc;

			//Cost
			int cost;

			//The associated sprite
			std::string sprite;

			//The size of sprite
			float size;

			//The color of the hat
			GLrgba color;

			HatInfo(){ cost = 1; size = 0.0f; }

			void Load(rapidxml::xml_node<char> *node)
			{
				LoadStr(name, "name", node);
				LoadStr(desc, "desc", node);
				LoadNum(cost, "cost", node);
				LoadStr(sprite, "sprite", node);
				LoadNum(size, "size", node);
				LoadColor(color, node);
			}
		};
		std::vector<HatInfo> hat_collection;

		//A default empty object that's used if a hat id is invalid
		HatInfo empty_hat;

		//The message for when you've already bought a hat at the store
		Button hat_purchased_message;

		//The ids for the hats you can buy right now
		int id_hat[STORE_HAT_SLOTS];

		//The first hat is free - this tooltip lets players know that
		std::string text_first;

		void Refresh(PlayerStats *p);
		HatInfo* HatFromID(int id)
		{
			if (id < hat_collection.size())
				return &hat_collection.at(id);

			//Return an empty object if not a valid id
			return &empty_hat;
		}

	public:
		HatShopMenu(){ last_world_loc_id = -1; refresh = true; }
		~HatShopMenu(){}

		void Init();
		void Reset() { refresh = true; last_world_loc_id = -1; }

		bool HandleEvents(const bool esc_key_pressed);
		void Draw();

		void Randomize();

		void SetUI();
	};
}