#pragma once

#include "common_header.h"
#include "Rectangle.h"
#include "render.h"

namespace pyrodactyl
{
	class Element : public pyroRect
	{
	protected:
		//This is the position(x,y) and size(w,h) of the element relative to the game's screen
		struct { Vector2f pos, size; } raw;

		//Which side of the screen is this object aligned to?
		TextAlign align;

		void BasicLoad(rapidxml::xml_node<char> *node, const bool &echo = true);

	public:
		Element() { align.x = ALIGN_LEFT; align.y = ALIGN_LEFT; }
		~Element() {}

		//Initialize an element from another
		void Init(const Element &e, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f);

		//The parent is the object inside which the element exists
		void Load(rapidxml::xml_node<char> *node, pyroRect *parent = nullptr, const bool &echo = true);

		void SetUI(pyroRect *parent = NULL);

		float RawPosX(){ return raw.pos.x; }
		float RawPosY(){ return raw.pos.y; }
	};
}