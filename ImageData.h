#pragma once

#include "common_header.h"
#include "ImageManager.h"
#include "element.h"

namespace pyrodactyl
{
	class ImageData : public Element
	{
		//The image
		ImageKey key;

	public:
		ImageData() { key = 0; }
		~ImageData(){}

		bool Valid() { return key != 0; }
		void Load(rapidxml::xml_node<char> *node, pyroRect *parent = nullptr, const bool &echo = true);

		void Draw(const SDL_Rect *clip = nullptr);
		void CircleDraw(const int &start_angle, const int &end_angle, const float &radius);
		void FadeDraw(const float &fade);
	};
}