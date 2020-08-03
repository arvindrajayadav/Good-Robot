#pragma once

#include "common_header.h"
#include "TextData.h"

namespace pyrodactyl
{
	//Tooltip for buttons
	class HoverInfo : public TextData
	{
	public:
		bool enabled;
		std::string text;

		HoverInfo(void){ enabled = false; }
		~HoverInfo(void){}

		void Load(rapidxml::xml_node<char> *node, pyroRect *parent = NULL);
		void Init(const HoverInfo &e, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f);

		void Draw();
	};
}
