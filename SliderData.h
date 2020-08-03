#pragma once

#include "common_header.h"
#include "ImageManager.h"
#include "ImageData.h"
#include "Caption.h"

namespace pyrodactyl
{
	//This represents the bar of the slider, while the button represents the slider knob
	class SliderData : public ImageData
	{
		//The value of the slider and the backup
		float value, backup;

		//The maximum and minimum values for the slider
		float max, min;

		//By how much is the slider value changed if you press left/right on keyboard/joystick
		float inc_val;

	public:
		SliderData(){ max = 1.0f; min = 0.0f; value = 0.5f; inc_val = 0.1f; backup = value; }
		~SliderData(){}

		void Load(rapidxml::xml_node<char> * node, pyroRect* parent = nullptr);

		void Clamp();

		float Inc() { return inc_val; }
		float Value() { return value; }

		void Value(const float val);
		void Value(const int &knob_x, const int &knob_w);

		int KnobPos(const int &knob_w);
		int Offset(const int &knob_w);

		void CreateBackup() { backup = value; }
		void RestoreBackup() { Value(backup); }
	};
}