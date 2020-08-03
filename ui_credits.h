#pragma once

#include "common_header.h"
#include "ImageData.h"
#include "TextData.h"
#include "TextManager.h"
#include "timer.h"
#include "ui_menu.h"

namespace pyrodactyl
{
	class CreditScreen
	{
		//Text and formatting information
		struct CreditText
		{
			//Title of the section (stuff like programmer, designer etc)
			std::string text;

			//The style it should be drawn in
			bool heading;
		};

		//The background image and company logo
		ImageData bg, logo;

		//The names displayed in the credits
		std::vector<CreditText> list;

		//The starting position of text
		Element start;

		//Text parameters
		struct TextParam
		{
			//By how much we increment the line
			Element inc;

			int color;
			FontKey font;
			TextAlign text_align;

			TextParam(){ color = 0; font = 1; }

			void Load(rapidxml::xml_node<char> *node)
			{
				if (NodeValid("inc", node))
					inc.Load(node->first_node("inc"));

				LoadNum(color, "color", node);
				LoadEnum(font, "font", node);
				LoadAlign(text_align.x, node, false, "text_align_x");
				LoadAlign(text_align.y, node, false, "text_align_y");
			}

			void SetUI() { inc.SetUI(); }
		} heading, paragraph;

		//The back button, website and twitter buttons
		ButtonMenu menu;

	public:
		CreditScreen(){}
		~CreditScreen(){}

		void Init();

		bool HandleEvents(bool esc_key_pressed);
		void Draw();

		void SetUI();
	};
}