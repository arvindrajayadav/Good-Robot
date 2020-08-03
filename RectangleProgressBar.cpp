#include "master.h"
#include "RectangleProgressBar.h"

using namespace pyrodactyl;

void RectangleProgressBar::Load(rapidxml::xml_node<char> *node)
{
	Element::Load(node);
	timer.Load(node, "delta_time");

	if (NodeValid("bg", node))
		bg.Load(node->first_node("bg"), this);

	if (NodeValid("caption", node))
		caption.Load(node->first_node("caption"), this);

	if (NodeValid("bar", node))
	{
		rapidxml::xml_node<char> *barnode = node->first_node("bar");

		if (NodeValid("max", barnode))
			max.Load(barnode->first_node("max"));

		if (NodeValid("cur", barnode))
			cur.Load(barnode->first_node("cur"));

		if (NodeValid("inc", barnode))
			change_inc.Load(barnode->first_node("inc"));

		if (NodeValid("dec", barnode))
			change_dec.Load(barnode->first_node("dec"));

		if (NodeValid("offset", barnode))
			offset.Load(barnode->first_node("offset"));
	}
}

void RectangleProgressBar::Draw(const int &value, const int &maximum, const char *title, bool draw_value)
{
	//Prevent divide by zero
	if (maximum == 0)
		return;

	bg.Draw();

	if (!init)
	{
		init = true;
		prev = value;
	}

	//Calculate pixels per unit according to the size IF the bar is of constant width
	pixels_per_unit = w / (float)maximum;

	//Draw the outline bar that depends on the maximum value of health
	max.Draw(x, y, h, pixels_per_unit, maximum, false);

	//Add the offset for all other bars
	int X = x + offset.x, Y = y + offset.y;

	if (prev > value)
	{
		//The value decreased, draw the decrease bar
		change_dec.Draw(X + static_cast<int>(value * pixels_per_unit), Y, h, pixels_per_unit, prev - value, true);

		//Decrease the bar value so it moves 1px forward every X ms, and eventually becomes equal to value
		if (timer.TargetReached())
		{
			prev -= 1 + static_cast<int>(pixels_per_unit / 2);
			if (prev < value)
				prev = value;

			//Reset the timer
			timer.Start();
		}

		//Draw the current bar
		cur.Draw(X, Y, h, pixels_per_unit, value, true);
	}
	else if (prev < value)
	{
		//Draw the current bar
		cur.Draw(X, Y, h, pixels_per_unit, value, true);

		//The value increased, draw the increase bar
		change_inc.Draw(X + static_cast<int>(prev * pixels_per_unit), Y, h, pixels_per_unit, value - prev, true);

		//Increase the bar value so it moves 1px forward every X ms, and eventually becomes equal to value
		if (timer.TargetReached())
		{
			prev += 1 + static_cast<int>(pixels_per_unit / 2);
			if (prev > value)
				prev = value;

			//Reset the timer
			timer.Start();
		}
	}
	else
		cur.Draw(X, Y, h, pixels_per_unit, value, true);

	//Draw the caption
	if (draw_value)
		caption.text = title + NumberToString<int>(value) +" / " + NumberToString<int>(maximum);
	else
		caption.text = title;

	caption.Draw();
}

void RectangleProgressBar::SetUI(pyroRect *parent)
{
	Element::SetUI(parent);
	bg.SetUI(this);
	caption.SetUI(this);
	offset.SetUI();

	max.SetUI();
	cur.SetUI();
	change_inc.SetUI();
	change_dec.SetUI();
}