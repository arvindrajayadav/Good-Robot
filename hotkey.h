#pragma once

#include "common_header.h"
#include "InputManager.h"
#include "element.h"

namespace pyrodactyl
{
	//This class is built to integrate the input check for hotkeys bound to buttons
	class HotKey
	{
		//The type of input the hotkey is checking for
		InputType input;

		//Used to draw the hotkey for certain controller buttons
		Element dim;

	public:
		HotKey() { input = CONTROL_NONE; }

		void Set(const InputType &val) { input = val; }
		const char* Name();

		void Load(rapidxml::xml_node<char> * node, pyroRect *parent = nullptr);
		bool HandleEvents();
		void Draw();

		void SetUI(pyroRect *parent = nullptr) { dim.SetUI(parent); }
	};
}