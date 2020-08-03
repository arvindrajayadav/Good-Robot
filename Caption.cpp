#include "master.h"
#include "Caption.h"

using namespace pyrodactyl;

void Caption::Load(rapidxml::xml_node<char> *node, pyroRect *parent)
{
	if (TextData::Load(node, parent, false))
	{
		LoadStr(text, "text", node);
		LoadNum(col_h, "color_h", node, false);
		LoadNum(col_s, "color_s", node, false);
		enabled = true;
	}
}

void Caption::Init(const Caption &e, pyroRect *parent, const float &XOffset, const float &YOffset)
{
	*this = e;
	Element::Init(e, parent, XOffset, YOffset);
}

void Caption::Draw(const CaptionState &state)
{
	if (enabled)
	{
		if (state == CAP_SELECT)
			TextData::Draw(text, col_s);
		else if (state == CAP_HOVER)
			TextData::Draw(text, col_h);
		else
			TextData::Draw(text);
	}
}