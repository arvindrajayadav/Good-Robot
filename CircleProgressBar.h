#pragma once

#include "ImageManager.h"
#include "timer.h"
#include "Caption.h"
#include "ImageData.h"

namespace pyrodactyl
{
	class CircleProgressBar : public Element
	{
		struct ImageSet
		{
			//The empty health bar - its value is equal to the maximum possible player health
			ImageKey max;

			//The current health bar - its value is equal to the current player health
			ImageKey cur;

			//The increase/decrease bars - drawn from the end point of the current bar to the previous value of the current bar
			ImageKey change_inc, change_dec;

			ImageSet(){ max = 0; cur = 0; change_inc = 0; change_dec = 0; }

			void Load(rapidxml::xml_node<char> *node)
			{
				LoadImgKey(max, "max", node);
				LoadImgKey(cur, "cur", node);
				LoadImgKey(change_inc, "inc", node);
				LoadImgKey(change_dec, "dec", node);
			}
		} img;

		//The background, always drawn
		ImageData bg;

		struct ArcData
		{
			//The scale of the bar - i.e., how big is the radius compared to the height of the screen
			//We use height instead of width coz height is generally lower
			float scale;

			//The radius of the bar
			float radius;

			ArcData() { scale = 0.2f; radius = 100.0f; }

			void Load(rapidxml::xml_node<char> *node)
			{
				LoadNum(scale, "scale", node);
				SetUI();
			}

			void SetUI() { radius = RenderViewportSize().y * scale; }
		} arc_bg, arc_bar;

		//Have we initialized the bar?
		bool init;

		//The old value of the bar, used to decide if the damage/change bar needs to be drawn
		int prev;

		//How much time one unit of health spends in transition from "damaged" state to "empty" state, stored in target_ticks of this timer
		//Similar to damage effect you see in Street Fighter health bars
		Timer timer;

		//The starting angle
		int start_angle;

		//The maximum angle (when the bar is 100% upgraded and full) - in degrees
		int arc_length;

	public:
		CircleProgressBar(){ init = false; start_angle = 0; arc_length = 350; }
		~CircleProgressBar(){}

		void Load(rapidxml::xml_node<char> *node);

		//value = current HP, current_maximum = current max HP, total_maximum = the maximum possible HP the player can upgrade to
		void Draw(const int &value, const int &current_maximum, const int &total_maximum);

		void SetUI(pyroRect *parent = nullptr);
	};
}