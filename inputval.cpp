#include "master.h"
#include "inputval.h"

using namespace pyrodactyl;

InputVal::InputVal()
{
	key_val = SDL_SCANCODE_UNKNOWN;
	key_alt = SDL_SCANCODE_UNKNOWN;
	inputs[0] = SDL_SCANCODE_UNKNOWN;
	inputs[1] = SDL_SCANCODE_UNKNOWN;
}

bool InputVal::Pressed()
{
	return InputKeyPressed(key_val) || InputKeyPressed(key_alt) || InputKeyPressed(inputs[0]) || InputKeyPressed(inputs[1]) || joy_ax.Pressed();
}

bool InputVal::State()
{
	return InputKeyState(key_val) || InputKeyState(key_alt) || InputKeyState(inputs[0]) || InputKeyState(inputs[1]) || joy_ax.State();
}

//------------------------------------------------------------------------
// Purpose: Load input values
//------------------------------------------------------------------------
void InputVal::LoadState(rapidxml::xml_node<char> *node)
{
	LoadStr(name, "name", node);
	LoadEnum(key_val, "key", node);
	LoadEnum(key_alt, "alt", node);
	LoadEnum(inputs[0], "input_0", node);
	LoadEnum(inputs[1], "input_1", node);

	if (NodeValid("axis", node))
		joy_ax.LoadState(node->first_node("axis"));
}

//------------------------------------------------------------------------
// Purpose: Save them
//------------------------------------------------------------------------
void InputVal::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char* title)
{
	rapidxml::xml_node<char> *child;
	child = doc.allocate_node(rapidxml::node_element, title);

	child->append_attribute(doc.allocate_attribute("name", name.c_str()));
	child->append_attribute(doc.allocate_attribute("key", gStrPool.Get(key_val)));
	child->append_attribute(doc.allocate_attribute("alt", gStrPool.Get(key_alt)));
	child->append_attribute(doc.allocate_attribute("input_0", gStrPool.Get(inputs[0])));
	child->append_attribute(doc.allocate_attribute("input_1", gStrPool.Get(inputs[1])));

	if (joy_ax.Valid())
		joy_ax.SaveState(doc, child);

	root->append_node(child);
}