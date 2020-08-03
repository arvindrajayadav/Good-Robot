#pragma once

#include "common_header.h"
#include "TextData.h"

namespace pyrodactyl
{
	enum CaptionState { CAP_NORMAL, CAP_SELECT, CAP_HOVER };

	//Caption for buttons
	class Caption : public TextData
	{
	public:
		bool enabled;
		int col_h, col_s;

		std::string text;

		Caption(void){ col_h = 0; col_s = 0; enabled = false; }
		~Caption(void){}

		void Load(rapidxml::xml_node<char> *node, pyroRect *parent = NULL);
		void Init(const Caption &e, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f);

		void Draw(const CaptionState &state = CAP_NORMAL);
	};
}
