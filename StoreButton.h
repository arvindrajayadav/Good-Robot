#pragma once

#include "button.h"
#include "ui_menu.h"
#include "ImageData.h"

namespace pyrodactyl
{
	//This is a button used to display one item in the store
	//The button itself is the outline image, the caption is the name of the item and the tooltip is the description
	class StoreButton : public Button
	{
		//The image of the item you're buying
		Element img;
		SpriteEntry sprite;
		GLrgba color;
		bool draw_img;

		//The text we want to draw as description
		std::string buy_disabled_text;

		//The buy button
		Button buy;

		//We need to draw the cost as a separate object, because otherwise setting buy button to invisible also
		//stops the cost from being drawn
		Caption price;

	public:

		StoreButton(){ draw_img = false; }
		~StoreButton(){}

		void Load(rapidxml::xml_node<char> *node, pyroRect *parent = nullptr, const bool &echo = true);
		void Init(const StoreButton &ref, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f);

		ButtonAction HandleEvents();
		void Draw();

		void SetCostText(bool condition, int cost, long player_money, SpriteEntry s = SPRITE_INVALID, GLrgba col = GLrgba(1, 1, 1));

		void SetUI(pyroRect *parent = nullptr);
	};
}