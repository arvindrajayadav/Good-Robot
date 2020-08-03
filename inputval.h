#pragma once

#include "common_header.h"
#include "loaders.h"
#include "XMLDoc.h"
#include "stringpool.h"
#include "input.h"
#include "inputaxis.h"

namespace pyrodactyl
{
	class InputVal
	{
	public:
		//Name of the key (such as "punch", "up")
		std::string name;

		//Keyboard keys
		SDL_Scancode key_val, key_alt;

		//Other inputs like mouse, controller or joystick button
		int inputs[2];

		//Controller or joystick axis
		InputAxisData joy_ax;

		InputVal();

		bool State();
		bool Pressed();
		float Delta() { return joy_ax.Delta(); }

		void LoadState(rapidxml::xml_node<char> *node);
		void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char* title);
	};
}