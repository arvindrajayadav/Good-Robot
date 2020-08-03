#include "master.h"
#include "TextData.h"

using namespace pyrodactyl;

bool TextData::Load(rapidxml::xml_node<char> *node, pyroRect *parent, const bool &echo)
{
	if (NodeValid(node, echo))
	{
		Element::Load(node, parent, echo);
		LoadEnum(font, "font", node);
		LoadNum(col, "color", node);
		LoadAlign(text_align.x, node, false, "text_align_x");
		LoadAlign(text_align.y, node, false, "text_align_y");
		LoadBool(background, "background", node, false);

		if (NodeValid("line", node))
		{
			rapidxml::xml_node<char> *linode = node->first_node("line");
			line.Load(linode);
			LoadNum(line_width, "width", linode);

			if (line_width <= 0)
				line_width = INT_MAX;
		}
		return true;
	}

	return false;
}

void TextData::Init(const TextData &e, pyroRect *parent, const float &XOffset, const float &YOffset)
{
	*this = e;
	Element::Init(e, parent, XOffset, YOffset);
	line.Init(e);
}

void TextData::Draw(const std::string &val)
{
	gTextManager.Draw(x, y, val, col, font, text_align, line_width, line.y, background);
}

void TextData::Draw(const std::string &val, const int &color, const bool &use_alpha, const float &alpha)
{
	gTextManager.Draw(x, y, val, color, font, text_align, line_width, line.y, background, use_alpha, alpha);
}