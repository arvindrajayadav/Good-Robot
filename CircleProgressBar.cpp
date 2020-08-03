#include "master.h"
#include "CircleProgressBar.h"

using namespace pyrodactyl;

const int DEGREES_IN_CIRCLE = 360;

void CircleProgressBar::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid("bg", node))
	{
		rapidxml::xml_node<char> *bgnode = node->first_node("bg");
		bg.Load(bgnode);
		arc_bg.Load(bgnode);
	}

	if (NodeValid("bar", node))
	{
		rapidxml::xml_node<char> *banode = node->first_node("bar");

		Element::Load(banode);
		timer.Load(banode, "delta_time");

		LoadNum(start_angle, "start_angle", banode);
		LoadNum(arc_length, "arc_length", banode);
		arc_bar.Load(banode);
	}

	if (NodeValid("img", node))
		img.Load(node->first_node("img"));
}

void CircleProgressBar::Draw(const int &value, const int &current_maximum, const int &total_maximum)
{
	//Prevent divide by zero
	if (total_maximum == 0)
		return;

	if (!init)
	{
		init = true;
		prev = value;
	}

	//Always draw the background
	bg.CircleDraw(start_angle, start_angle + DEGREES_IN_CIRCLE, arc_bg.radius);

	//Find the angle of maximum health
	int max_angle = start_angle + ((current_maximum * arc_length) / total_maximum);
	gImageManager.CircleDraw(x, y, img.max, start_angle, max_angle, arc_bar.radius);

	//Find the angle of current health
	int cur_angle = start_angle + ((value * arc_length) / total_maximum);
	gImageManager.CircleDraw(x, y, img.cur, start_angle, cur_angle, arc_bar.radius);

	if (prev > value)
	{
		//The value decreased, draw the decrease bar
		int prev_angle = start_angle + ((prev * arc_length) / total_maximum);
		gImageManager.CircleDraw(x, y, img.change_dec, cur_angle, prev_angle, arc_bar.radius);

		//Decrease prev once every X ms, so it eventually becomes equal to value
		if (timer.TargetReached())
		{
			--prev;

			//Reset the timer
			timer.Start();
		}
	}
	else if (prev < value)
	{
		//The value increased, draw the increase bar
		int prev_angle = start_angle + ((prev * arc_length) / total_maximum);
		gImageManager.CircleDraw(x, y, img.change_inc, prev_angle, cur_angle, arc_bar.radius);

		//Increase prev once every X ms, so it eventually becomes equal to value
		if (timer.TargetReached())
		{
			++prev;

			//Reset the timer
			timer.Start();
		}
	}
}

void CircleProgressBar::SetUI(pyroRect *parent)
{
	bg.SetUI(parent);
	Element::SetUI(parent);
	arc_bg.SetUI();
	arc_bar.SetUI();
}