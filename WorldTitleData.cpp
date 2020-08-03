#include "master.h"
#include "WorldTitleData.h"
#include "world.h"

using namespace pyrodactyl;

void WorldTitleData::SetLevelTitle()
{
	WorldTitle(title.text, subtitle.text, fade);
}

void WorldTitleData::Load(rapidxml::xml_node<char> *node, pyroRect *parent, const bool &echo)
{
	ImageData::Load(node, parent, echo);

	if (NodeValid("title", node))
	{
		rapidxml::xml_node<char> *tinode = node->first_node("title");
		title.Load(tinode, this);
		LoadStr(title.text, "text", tinode, false);
	}

	if (NodeValid("subtitle", node))
	{
		rapidxml::xml_node<char> *subnode = node->first_node("subtitle");
		subtitle.Load(subnode, this);
		LoadStr(subtitle.text, "text", subnode, false);
	}
}

void WorldTitleData::Draw()
{
	ImageData::FadeDraw(fade);
	title.Draw(title.text, title.col, true, fade);
	subtitle.Draw(subtitle.text, subtitle.col, true, fade);
}

void WorldTitleData::SetUI()
{
	ImageData::SetUI();
	title.SetUI(this);
	subtitle.SetUI(this);
}