#pragma once

#include "common_header.h"
#include "ImageManager.h"

namespace pyrodactyl
{
	struct ButtonImage
	{
		ImageKey normal, select, hover;

		bool operator== (const ButtonImage &img){ return normal == img.normal && select == img.select && hover == img.hover; }

		ButtonImage() { normal = 0; select = 0; hover = 0; }

		void Load(rapidxml::xml_node<char> *node, const bool &echo = true)
		{
			if (NodeValid(node))
			{
				LoadImgKey(normal, "img_b", node, echo);
				LoadImgKey(select, "img_s", node, echo);
				LoadImgKey(hover, "img_h", node, echo);
			}
		}

		void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
		{
			root->append_attribute(doc.allocate_attribute("img_b", gStrPool.Get(normal)));
			root->append_attribute(doc.allocate_attribute("img_s", gStrPool.Get(select)));
			root->append_attribute(doc.allocate_attribute("img_h", gStrPool.Get(hover)));
		}
	};

	//State buttons require two image sets
	struct StateButtonImage
	{
		ButtonImage normal, select;

		StateButtonImage() {}
		StateButtonImage(rapidxml::xml_node<char> *node, const bool &echo = true) { Load(node, echo); }

		void Load(rapidxml::xml_node<char> *node, const bool &echo = true)
		{
			if (NodeValid("normal", node, echo))
				normal.Load(node->first_node("normal"), echo);

			if (NodeValid("select", node, false))
				select.Load(node->first_node("select"), echo);
			else
				select = normal;
		}
	};
}