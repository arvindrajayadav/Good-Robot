#include "master.h"
#include "RadioButtonData.h"

using namespace pyrodactyl;

void RadioButtonData::Load(rapidxml::xml_node<char> * node, pyroRect *parent)
{
	LoadImgKey(on, "on", node);
	LoadImgKey(off, "off", node);
	Element::Load(node, parent);

	if (NodeValid("offset", node))
		offset.Load(node->first_node("offset"));
}

void RadioButtonData::Draw(const int &x, const int &y)
{
	if (state)
		gImageManager.Draw(x + offset.x, y + offset.y, w, h, on);
	else
		gImageManager.Draw(x + offset.x, y + offset.y, w, h, off);
}