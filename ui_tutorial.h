#pragma once

#include "ValuePicker.h"

namespace pyrodactyl
{
	//Display several tooltips to the player
	class TutorialDisplay
	{
		//The player can cycle between 3 lines of news text
		struct Tutorial
		{
			std::string text;

			void Load(rapidxml::xml_node<char> *node)
			{
				LoadStr(text, "val", node);
			}
		};
		ValuePicker<Tutorial> ticker;

		//The description of the tutorial
		HoverInfo caption;

		//The text used for the tutorial
		std::vector<std::string> text;

	public:
		void Load(rapidxml::xml_node<char> *node);
		bool HandleEvents();
		void Draw();
		void SetUI();
	};
}