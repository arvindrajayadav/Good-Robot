#include "master.h"
#include "ui_tutorial.h"

using namespace pyrodactyl;

void TutorialDisplay::Load(rapidxml::xml_node<char> *node)
{
	ticker.Load(node);

	if (NodeValid("caption", node))
		caption.Load(node->first_node("caption"));
}

bool TutorialDisplay::HandleEvents()
{
	return ticker.HandleEvents();
}

void TutorialDisplay::Draw()
{
	caption.Draw();
	ticker.Draw();
}

void TutorialDisplay::SetUI()
{
	caption.SetUI();
	ticker.SetUI();
}