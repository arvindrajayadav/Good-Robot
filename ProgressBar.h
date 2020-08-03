#pragma once

#include "common_header.h"
#include "ImageManager.h"
#include "timer.h"
#include "Caption.h"

namespace pyrodactyl
{
	class ProgressBar
	{
		//The image for this bar
		struct
		{
			ImageKey img;
			int w, h;

			void Load(rapidxml::xml_node<char> *node, const char* name)
			{
				LoadImgKey(img, name, node);
				w = gImageManager.GetTexture(img)->Size().x;
				h = gImageManager.GetTexture(img)->Size().y;
			}
		} start, mid, end;

		//The offset at which every bar other than health is drawn
		Element offset_end;

	public:
		ProgressBar() {}

		void Load(rapidxml::xml_node<char> *node)
		{
			start.Load(node, "img_start");
			mid.Load(node, "img_mid");
			end.Load(node, "img_end");

			if (NodeValid("offset_end", node))
				offset_end.Load(node->first_node("offset_end"));
		}

		//Draw function
		//Is the end bit a part of the value itself? true = health bar, false = health bar container
		void Draw(int x, int y, int h, float pixels_per_unit, int val, bool end_incl_in_val);

		void SetUI() { offset_end.SetUI(); }
	};
}
