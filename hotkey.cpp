#include "master.h"
#include "hotkey.h"

using namespace pyrodactyl;

void HotKey::Load(rapidxml::xml_node<char> * node, pyroRect *parent)
{
	LoadEnum(input, "input", node);

	if (NodeValid("dim", node))
		dim.Load(node->first_node("dim"), parent);
}

bool HotKey::HandleEvents()
{
	if (input > CONTROL_NONE && input < CONTROL_COUNT)
		return gInput.Pressed(input);

	return false;
}

const char* HotKey::Name()
{
	if (input > CONTROL_NONE && input < CONTROL_COUNT)
		return SDL_GetScancodeName(gInput.iv[input].key_val);

	return "";
}

void HotKey::Draw()
{
	//Only draw hotkey if a controller is active
	if (InputJoystickActive())
		gInput.Draw(input, dim.x, dim.y, dim.w, dim.h);
}