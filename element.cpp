#include "master.h"
#include "element.h"

using namespace pyrodactyl;

void Element::Init(const Element &e, pyroRect *parent, const float &XOffset, const float &YOffset)
{
	*this = e;
	raw.pos.x += XOffset;
	raw.pos.y += YOffset;
	SetUI(parent);
}

void Element::BasicLoad(rapidxml::xml_node<char> *node, const bool &echo)
{
	raw.pos.Load(node, echo);
	raw.size.Load(node, echo, "w", "h");
	LoadAlign(align.x, node, echo, "align_x");
	LoadAlign(align.y, node, echo, "align_y");
}

void Element::Load(rapidxml::xml_node<char> *node, pyroRect *parent, const bool &echo)
{
	BasicLoad(node, echo);
	SetUI(parent);
}

void Element::SetUI(pyroRect *parent)
{
	//Dimensions are relative to screen height
	w = static_cast<int>(RenderViewportSize().y * raw.size.x);
	h = static_cast<int>(RenderViewportSize().y * raw.size.y);

	if (parent == NULL)
	{
		//Position is relative to screen
		x = static_cast<int>(RenderViewportSize().y * raw.pos.x);
		y = static_cast<int>(RenderViewportSize().y * raw.pos.y);

		switch (align.x)
		{
		case ALIGN_CENTER: x += (RenderViewportSize().x - w) / 2; break;
		case ALIGN_RIGHT: x += RenderViewportSize().x - w; break;
		default: break;
		}

		switch (align.y)
		{
		case ALIGN_CENTER: y += (RenderViewportSize().y - h) / 2; break;
		case ALIGN_RIGHT: y += RenderViewportSize().y - h; break;
		default: break;
		}
	}
	else
	{
		//Position is relative to screen
		x = parent->x + static_cast<int>(RenderViewportSize().y * raw.pos.x);
		y = parent->y + static_cast<int>(RenderViewportSize().y * raw.pos.y);

		switch (align.x)
		{
		case ALIGN_CENTER: x += (parent->w - w) / 2; break;
		case ALIGN_RIGHT: x += parent->w - w; break;
		default: break;
		}

		switch (align.y)
		{
		case ALIGN_CENTER: y += (parent->h - h) / 2; break;
		case ALIGN_RIGHT: y += parent->h - h; break;
		default: break;
		}
	}
}