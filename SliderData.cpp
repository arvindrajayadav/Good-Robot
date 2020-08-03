#include "master.h"
#include "SliderData.h"

using namespace pyrodactyl;

void SliderData::Load(rapidxml::xml_node<char> * node, pyroRect* parent)
{
	ImageData::Load(node, parent);
	LoadNum(min, "min", node);
	LoadNum(max, "max", node);

	//By default, 10 presses will get you from slider min->max and max->min
	if (!LoadNum(inc_val, "inc_val", node))
		inc_val = (max - min) / 10.0f;

	CreateBackup();
}

void SliderData::Clamp()
{
	if (value < min)
		value = min;
	else if (value > max)
		value = max;
}

void SliderData::Value(const float val)
{
	value = val;
	Clamp();
}

void SliderData::Value(const int &knob_x, const int &knob_w)
{
	float pos = (float)(knob_x - x) / (float)(w - knob_w);
	value = min + ((max - min) * pos);
	Clamp();
}

int SliderData::KnobPos(const int &knob_w)
{
	return static_cast<int>((static_cast<float>(w - knob_w) * value) / (max - min));
}

int SliderData::Offset(const int &knob_w)
{
	return static_cast<int>((static_cast<float>(w - knob_w) * (value - min)) / (max - min));
}