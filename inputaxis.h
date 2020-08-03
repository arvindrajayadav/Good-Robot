#pragma once

#include "common_header.h"
#include "loaders.h"
#include "XMLDoc.h"
#include "stringpool.h"
#include "input.h"

namespace pyrodactyl
{
	//Controller or joystick axis
	class InputAxisData
	{
		//The axis
		int id;

		//Value of controller axis - because controller axes have a range of values
		int val;

		//Do we want to do a "less than" or "greater than" comparison?
		bool greater;

		//For menus, we want to consider a "flick" of the analog stick - i.e. a back and forth movement
		//This means we toggle this flag once the stick hits maximum value, then check if the stick has
		//gone back to its previous value
		bool toggle;

	public:
		InputAxisData() { id = -1; val = 0; toggle = false; greater = false; }

		bool Pressed();
		bool State();
		float Delta() { return InputAxisf(id); }
		bool Valid() { return id >= 0; }

		void LoadState(rapidxml::xml_node<char> *node);
		void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	};
}