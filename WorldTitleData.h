#pragma once

#include "common_header.h"
#include "ImageData.h"
#include "TextData.h"
#include "world.h"

namespace pyrodactyl
{
	struct WorldTitleData : public ImageData
	{
		struct Text : public TextData
		{
			std::string text;
		} title, subtitle;

		float fade;

		WorldTitleData() { fade = 0.0f; }

		//Load title and subtitle from the level information
		void SetLevelTitle();

		void Load(rapidxml::xml_node<char> *node, pyroRect *parent = nullptr, const bool &echo = true);
		void Draw();
		void SetUI();
	};
}