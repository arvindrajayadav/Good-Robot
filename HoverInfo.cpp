#include "master.h"
#include "HoverInfo.h"

using namespace pyrodactyl;

void HoverInfo::Load(rapidxml::xml_node<char> *node, pyroRect *parent)
{
	if (TextData::Load(node, parent, false))
	{
		LoadStr(text, "text", node);
		enabled = true;
	}
}

void HoverInfo::Init(const HoverInfo &e, pyroRect *parent, const float &XOffset, const float &YOffset)
{
	*this = e;
	Element::Init(e, parent, XOffset, YOffset);
}

void HoverInfo::Draw()
{
	if (enabled)
		TextData::Draw(text);
}