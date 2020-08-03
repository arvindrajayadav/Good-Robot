#pragma once

#include "common_header.h"
#include "ImageManager.h"
#include "vectors.h"
#include "element.h"

namespace pyrodactyl
{
	//This button has two states that can be switched between by clicking the button
	class RadioButtonData : public Element
	{
		//The images corresponding to the state
		ImageKey on, off;

		//The offset at which the on/off image is drawn
		Element offset;

	public:
		//The state of the button - true is on, false is off
		bool state;

		RadioButtonData(){ state = false; on = 0; off = 0; }
		~RadioButtonData(){}

		void ToggleState() { state = !state; }

		void Load(rapidxml::xml_node<char> * node, pyroRect *parent = nullptr);
		void Draw(const int &x, const int &y);
	};
}