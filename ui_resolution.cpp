#include "master.h"
#include "system.h"
#include "ui_resolution.h"
#include "menu.h"

using namespace pyrodactyl;

void ResolutionMenu::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid("bg", node))
	{
		rapidxml::xml_node<char> *bgnode = node->first_node("bg");

		if (NodeValid("menu", bgnode))
			bg_res.Load(bgnode->first_node("menu"));

		if (NodeValid("confirm", bgnode))
			bg_confirm.Load(bgnode->first_node("confirm"));
	}

	if (NodeValid("title", node))
		title.Load(node->first_node("title"));

	if (NodeValid("info", node))
	{
		res_info.Load(node->first_node("info"));
		res_prefix = res_info.text;
	}

	if (NodeValid("confirm", node))
		me_confirm.Load(node->first_node("confirm"));

	if (NodeValid("reference", node))
		ref.Load(node->first_node("reference"));

	if (NodeValid("inc", node))
		inc.Load(node->first_node("inc"));

	if (NodeValid("back", node))
		back.Load(node->first_node("back"));

	if (NodeValid("message", node))
		msg.Load(node->first_node("message"));

	if (NodeValid("countdown", node))
	{
		rapidxml::xml_node<char> *counode = node->first_node("countdown");
		countdown.Load(counode);
		timer.Load(counode, "time");
		LoadStr(prefix, "prefix", counode);
	}

	//We're just loading the grid parameters and row value
	if (NodeValid("menu", node))
		me_res.Load(node->first_node("menu"));

	//The buttons here don't matter, they'll be populated based on resolution
	me_res.Clear();

	//Load the list of allowed resolutions here
	//SDL_Rect** modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_OPENGL);
	for (int i = 0; i < SystemResolutions(); i++)
	{
		Button b;
		GLcoord2	size = SystemResolution(i);
		b.Init(ref, nullptr, inc.RawPosX() * (i / me_res.Rows()), inc.RawPosY() * (i % me_res.Rows()));
		b.caption.text = NumberToString<int>(size.x) + " x " + NumberToString<int>(size.y);
		me_res.element.push_back(b);

		if (size.x == RenderViewportSize().x && size.y == RenderViewportSize().y)
			cur_index = i;

		prev_index = cur_index;
	}

	res_info.text = res_prefix + NumberToString(RenderViewportSize().x) + " x " + NumberToString(RenderViewportSize().y);

	me_res.AssignPaths();
}

void ResolutionMenu::ResolutionSet(int val)
{
	prev_index = cur_index;
	cur_index = val;
	SystemResolutionSet(cur_index);

	res_info.text = res_prefix + NumberToString(RenderViewportSize().x) + " x " + NumberToString(RenderViewportSize().y);
}

void ResolutionMenu::Draw()
{
	switch (state)
	{
	case STATE_NORMAL:
		bg_res.Draw();
		title.Draw();
		res_info.Draw();
		me_res.Draw();
		back.Draw();
		break;
	case STATE_CONFIRM:
		bg_confirm.Draw();
		msg.Draw();
		countdown.Draw(prefix + NumberToString(timer.RemainingTicks() / 1000));
		me_confirm.Draw();
		break;
	default:break;
	}
}

bool ResolutionMenu::HandleEvents(const bool game_running)
{
	switch (state)
	{
	case STATE_NORMAL:
	{
		int choice = me_res.HandleEvents();
		if (choice > -1)
		{
			//Set current resolution to index, switch state to confirm
			ResolutionSet(choice);

			if (game_running)
			{
				//Reset state to pause menu open if we changed resolution from
				//inside the game
				MenuOpen(MENU_MAIN);
			}

			timer.Start();
			state = STATE_CONFIRM;
		}

		if (back.HandleEvents())
			return true;
	}
	break;
	case STATE_CONFIRM:
	{
		int choice = me_confirm.HandleEvents();
		if (choice == 0)
		{
			//Accept the new resolution
			state = STATE_NORMAL;
			timer.Stop();
			return true;
		}
		else if (choice == 1 || timer.TargetReached())
		{
			//Revert to the old resolution
			ResolutionSet(prev_index);
			timer.Stop();
			state = STATE_NORMAL;
		}
	}
	break;
	default:break;
	}

	return false;
}

void ResolutionMenu::SetUI()
{
	ref.SetUI();
	inc.SetUI();
	countdown.SetUI();

	title.SetUI();
	res_info.SetUI();

	bg_res.SetUI();
	me_res.SetUI();
	back.SetUI();

	bg_confirm.SetUI();
	me_confirm.SetUI();
	msg.SetUI();
}