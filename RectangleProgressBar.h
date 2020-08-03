#pragma once

#include "ImageManager.h"
#include "timer.h"
#include "Caption.h"
#include "ProgressBar.h"
#include "ImageData.h"

namespace pyrodactyl
{
	//A health bar has variable width that changes according to player stats
	class RectangleProgressBar : public Element
	{
	protected:
		//The background image
		ImageData bg;

		//The empty health bar - its value is equal to the maximum possible player health
		ProgressBar max;

		//The current health bar - its value is equal to the current player health
		ProgressBar cur;

		//The increase/decrease bars - drawn from the end point of the current bar to the previous value of the current bar
		ProgressBar change_inc, change_dec;

		//1 unit of val = pixels_per_unit pixels of image, can be less than 1
		float pixels_per_unit;

		//The old value of the bar, used to decide if the damage/change bar needs to be drawn
		int prev;

		//How much time one unit of health spends in transition from "damaged" state to "empty" state, stored in target_ticks of this timer
		//Similar to damage effect you see in Street Fighter health bars
		Timer timer;

		//Have we initialized the bar?
		bool init;

		//The offset at which every bar other than max is drawn
		Element offset;

		//Used to draw text like this: "XP: Value / Maximum"
		Caption caption;

	public:
		RectangleProgressBar(){ Reset(); prev = 0; pixels_per_unit = 1; }
		~RectangleProgressBar(){}

		void Reset() { init = false; }

		void Load(rapidxml::xml_node<char> *node);
		void Draw(const int &value, const int &maximum, const char *title, bool draw_value);

		void SetUI(pyroRect *parent = nullptr);
	};
}