#include "master.h"
#include "InputManager.h"
#include "system.h"
#include "texture.h"
#include "render.h"
#include "element.h"

namespace pyrodactyl
{
	InputManager gInput;
}

using namespace pyrodactyl;
using namespace boost::filesystem;

//------------------------------------------------------------------------
// Purpose: Return pressed/depressed state of key
//------------------------------------------------------------------------
const bool InputManager::State(const InputType &val)
{
	return iv[val].State();
}

//------------------------------------------------------------------------
// Purpose: Return position of joystick
//------------------------------------------------------------------------
const float InputManager::Delta(const InputType &val)
{
	return iv[val].Delta();
}

//------------------------------------------------------------------------
// Purpose: Check for controller and keyboard input simultaneously
//------------------------------------------------------------------------
const bool InputManager::Pressed(const InputType &val)
{
	return iv[val].Pressed();
}

//------------------------------------------------------------------------
// Purpose: Used for "use right analog stick to fire"
//------------------------------------------------------------------------
bool InputManager::RightAxisFire()
{
	if (InputJoystickActive())
	{
		if (InputAxisNeutral(JOY_RIGHT_STICK_X) && InputAxisNeutral(JOY_RIGHT_STICK_Y))
			return false;

		return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Load from file
//------------------------------------------------------------------------
void InputManager::Init()
{
	const std::string DEFAULT_FILENAME = "core/data/controls.xml";

	std::string filename = SystemSavePath();
	filename += "controls.xml";

	if (!is_regular_file(filename))
	{
		//The other file does not exist, just use the default file
		Load(DEFAULT_FILENAME);
	}
	else
	{
		//We are using the other file, check if it is up to date or not
		if (Version(DEFAULT_FILENAME) > Version(filename))
		{
			//The game has been updated to a different control scheme, use the default file
			Load(DEFAULT_FILENAME);
		}
		else
		{
			//The version set by the player is fine, just use that
			Load(filename);
		}
	}

	XMLDoc image_list("core/data/hotkey_img.xml");
	if (image_list.ready())
	{
		rapidxml::xml_node<char> *node = image_list.Doc()->first_node("res");

		if (NodeValid("a", node))
			LoadStr(hotkey.a, "sprite", node->first_node("a"));

		if (NodeValid("b", node))
			LoadStr(hotkey.b, "sprite", node->first_node("b"));

		if (NodeValid("x", node))
			LoadStr(hotkey.x, "sprite", node->first_node("x"));

		if (NodeValid("y", node))
			LoadStr(hotkey.y, "sprite", node->first_node("y"));

		if (NodeValid("lb", node))
			LoadStr(hotkey.lb, "sprite", node->first_node("lb"));

		if (NodeValid("rb", node))
			LoadStr(hotkey.rb, "sprite", node->first_node("rb"));

		if (NodeValid("lt", node))
			LoadStr(hotkey.lt, "sprite", node->first_node("lt"));

		if (NodeValid("rt", node))
			LoadStr(hotkey.rt, "sprite", node->first_node("rt"));

		if (NodeValid("ls", node))
			LoadStr(hotkey.ls, "sprite", node->first_node("ls"));

		if (NodeValid("rs", node))
			LoadStr(hotkey.rs, "sprite", node->first_node("rs"));

		if (NodeValid("back", node))
			LoadStr(hotkey.back, "sprite", node->first_node("back"));

		if (NodeValid("start", node))
			LoadStr(hotkey.start, "sprite", node->first_node("start"));

		if (NodeValid("down", node))
			LoadStr(hotkey.down, "sprite", node->first_node("down"));

		if (NodeValid("left", node))
			LoadStr(hotkey.left, "sprite", node->first_node("left"));

		if (NodeValid("right", node))
			LoadStr(hotkey.right, "sprite", node->first_node("right"));

		if (NodeValid("up", node))
			LoadStr(hotkey.up, "sprite", node->first_node("up"));
	}

	CreateBackup();
}

//------------------------------------------------------------------------
// Purpose: Draw hotkey images
//------------------------------------------------------------------------
void InputManager::Draw(const InputType &type, const int &x, const int &y, const int &w, const int &h)
{
	switch (type)
	{
	case CONTROL_UP: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.up)); break;
	case CONTROL_LEFT: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.left)); break;
	case CONTROL_DOWN: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.down)); break;
	case CONTROL_RIGHT: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.right)); break;
	case CONTROL_ACTIVATE: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.a)); break;
	case CONTROL_PREV: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.lb)); break;
	case CONTROL_NEXT: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.rb)); break;
	case CONTROL_BACK: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.b)); break;
	case CONTROL_PAUSE: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.start)); break;
	case CONTROL_LEADERBOARD_PREV: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.x)); break;
	case CONTROL_LEADERBOARD_NEXT: RenderSprite(x, y, w, h, SpriteEntryLookup(hotkey.y)); break;
	default: break;
	}
}

//------------------------------------------------------------------------
// Purpose: Load key & controller binding settings from file
//------------------------------------------------------------------------
void InputManager::Load(const std::string &filename)
{
	XMLDoc control_list(filename);
	if (control_list.ready())
	{
		rapidxml::xml_node<char> *node = control_list.Doc()->first_node("controls");
		if (NodeValid(node))
		{
			LoadNum(version, "version", node);

			int i = 0;
			for (auto n = node->first_node(); n != nullptr && i < CONTROL_COUNT; n = n->next_sibling(), ++i)
				iv[i].LoadState(n);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Create and restore backup
//------------------------------------------------------------------------
void InputManager::CreateBackup()
{
	for (int i = 0; i < CONTROL_COUNT; ++i)
		backup[i] = iv[i];
}

void InputManager::RestoreBackup()
{
	for (int i = 0; i < CONTROL_COUNT; ++i)
		iv[i] = backup[i];
}

//------------------------------------------------------------------------
// Purpose: Save to file
//------------------------------------------------------------------------
void InputManager::Save()
{
	rapidxml::xml_document<char> doc;

	std::string filename = SystemSavePath();
	filename += "controls.xml";

	// xml declaration
	rapidxml::xml_node<char> *decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	// root node
	rapidxml::xml_node<char> *root = doc.allocate_node(rapidxml::node_element, "controls");
	root->append_attribute(doc.allocate_attribute("version", gStrPool.Get(version)));
	for (int i = 0; i < CONTROL_COUNT; i++)
		iv[i].SaveState(doc, root, "i");

	doc.append_node(root);
	std::string xml_as_string;
	rapidxml::print(std::back_inserter(xml_as_string), doc);

	std::ofstream save(filename, std::ios::out);
	if (save.is_open())
	{
		save << xml_as_string;
		save.close();
	}

	doc.clear();
	CreateBackup();
}