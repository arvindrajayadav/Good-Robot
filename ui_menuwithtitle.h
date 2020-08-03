#pragma once

#include "ImageData.h"
#include "ui_menu.h"

namespace pyrodactyl
{
	struct MenuWithBgandTitle
	{
		//The background
		ImageData bg;

		//The title
		Caption title;

		//The menu,
		ButtonMenu menu;

		void Load(rapidxml::xml_node<char> *node)
		{
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			if (NodeValid("title", node))
				title.Load(node->first_node("title"));

			if (NodeValid("menu", node))
				menu.Load(node->first_node("menu"));
		}

		void Draw() { bg.Draw(); title.Draw(); menu.Draw(); }
		void SetUI() { bg.SetUI(); title.SetUI(); menu.SetUI(); }
	};
}