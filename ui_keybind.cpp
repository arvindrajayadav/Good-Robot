#include "master.h"
#include "ui_keybind.h"

using namespace pyrodactyl;

void KeyBindMenu::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid("bg", node))
		bg.Load(node->first_node("bg"));

	if (NodeValid("title", node))
		title.Load(node->first_node("title"));

	if (NodeValid("menu", node))
	{
		rapidxml::xml_node<char> *menode = node->first_node("menu");

		if (NodeValid("ref", menode))
			ref.Load(menode->first_node("ref"));

		if (NodeValid("settings", menode))
			menu.Load(menode->first_node("settings"));

		menu.Rows(CONTROL_REBINDABLE_COUNT);

		if (NodeValid("prompt", menode))
			prompt.Load(menode->first_node("prompt"));

		if (NodeValid("inc", menode))
			inc.Load(menode->first_node("inc"));
	}

	if (NodeValid("back", node))
		back.Load(node->first_node("back"));

	//Initialize the menu
	InitMenu();
}

void KeyBindMenu::InitMenu()
{
	int start = 0, size = CONTROL_REBINDABLE_COUNT, r = menu.Rows();

	for (int i = 0; i < size * 2; i++)
	{
		Button b;
		b.Init(ref, nullptr, inc.RawPosX() * (i / r), inc.RawPosY() * (i % r));

		if (i < r)
		{
			b.caption.text = SDL_GetScancodeName(gInput.iv[start + (i % r)].key_val);
			b.desc.text = gInput.iv[start + (i % r)].name;
		}
		else
		{
			b.caption.text = SDL_GetScancodeName(gInput.iv[start + (i % r)].key_alt);
			b.desc.enabled = false;
		}

		menu.element.push_back(b);
	}

	menu.AssignPaths();
}

void KeyBindMenu::SetCaption()
{
	int start = 0, size = CONTROL_REBINDABLE_COUNT, r = menu.Rows();

	for (int i = 0; i < size * 2; i++)
	{
		if (i < r)
			menu.element.at(i).caption.text = SDL_GetScancodeName(gInput.iv[start + (i % r)].key_val);
		else
			menu.element.at(i).caption.text = SDL_GetScancodeName(gInput.iv[start + (i % r)].key_alt);
	}
}

bool KeyBindMenu::HandleEvents()
{
	switch (state)
	{
	case STATE_NORMAL:
		choice = menu.HandleEvents();
		if (choice >= 0)
		{
			prompt.Swap(menu.element.at(choice).caption);
			state = STATE_KEY;
			break;
		}

		break;
	case STATE_KEY:
		if (InputAnyKeyPressed())
		{
			int key = InputLastPressed();
			//Cutoff for valid values and to avoid stuff like setting left click, sleep or monitor switch button as input
			//230 = Right Alt in SDL_Scancode.h
			if (key > 0 && key < 230)
			{
				SwapKey(static_cast<SDL_Scancode>(key));
				SetCaption();
				menu.element.at(choice).caption.col = prompt.col_prev;
				state = STATE_NORMAL;
			}
		}
		break;
	default:break;
	}

	return (back.HandleEvents() == BUAC_LCLICK);
}

void KeyBindMenu::Draw()
{
	bg.Draw();
	title.Draw();
	menu.Draw();
	back.Draw();
}

void KeyBindMenu::SwapKey(const SDL_Scancode &find)
{
	int start = 0, size = CONTROL_REBINDABLE_COUNT, r = menu.Rows();
	int pos = start + (choice % r);

	for (int i = start; i < size; ++i)
	{
		if (gInput.iv[i].key_val == find)
		{
			gInput.iv[i].key_val = gInput.iv[pos].key_val;
			break;
		}
		else if (gInput.iv[i].key_alt == find)
		{
			gInput.iv[i].key_alt = gInput.iv[pos].key_val;
			break;
		}
	}

	if (choice < r)
		gInput.iv[pos].key_val = find;
	else
		gInput.iv[pos].key_alt = find;
}

void KeyBindMenu::SetUI()
{
	menu.Clear();

	ref.SetUI();
	inc.SetUI();
	InitMenu();

	bg.SetUI();
	title.SetUI();
	back.SetUI();
}