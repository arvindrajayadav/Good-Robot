#pragma once

#include "ui_menu.h"
#include "StoreButton.h"
#include "player.h"
#include "ui_news.h"

#define STORE_WEAPON_SLOTS 2

namespace pyrodactyl
{
	class StoreMenu
	{
		//The background
		ImageData bg;

		//The title of the store
		Caption title;

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

		//See if player health has changed since they last opened this menu
		int prev_shields;

		//The ids for the weapons you can buy right now
		int id_prim[STORE_WEAPON_SLOTS], id_sec[STORE_WEAPON_SLOTS];

		//Keep track of where the last store we opened was
		//if it is the same location as the current one, don't randomize weapons
		int last_world_loc_id;

		//The tooltip for the description of stuff you can buy at the store
		HoverInfo tooltip;

		//The news displayed on screen
		NewsDisplay news;

		//Descriptions for the warranty and repair options
		struct { std::string warranty, repair; } desc;

		void Refresh(PlayerStats *p);

	public:
		StoreMenu();
		~StoreMenu(){}

		void Init();
		void Reset() { refresh = true; last_world_loc_id = -1; }

		bool HandleEvents(const bool esc_key_pressed);
		void Draw();

		void Randomize();
		void SpawnWeapon(int weapon_id);

		void SetUI();
	};
}