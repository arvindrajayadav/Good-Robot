#pragma once

#include "common_header.h"
#include "TextManager.h"
#include "element.h"

namespace pyrodactyl
{
	class TextData : public Element
	{
	public:
		int col;
		FontKey font;
		bool background;
		TextAlign text_align;

		//X represents the number of characters before a line break
		int line_width;

		//We only use the y coordinate of this to know the position of the next line
		Element line;

		TextData(void){ col = 0; font = FONT_MENU; align.x = ALIGN_LEFT; align.y = ALIGN_LEFT; background = false; line_width = INT_MAX; }
		~TextData(void){}

		bool Load(rapidxml::xml_node<char> *node, pyroRect *parent = NULL, const bool &echo = true);
		void Init(const TextData &e, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f);

		//Plain drawing
		void Draw(const std::string &val);

		//Draw with a different color and alpha
		void Draw(const std::string &val, const int &color, const bool &use_alpha = false, const float &alpha = 1.0f);
	};
}