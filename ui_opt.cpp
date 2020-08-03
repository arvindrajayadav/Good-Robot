#include "master.h"
#include "ui_opt.h"
#include "env.h"
#include "system.h"
#include "steam_data.h"
#include "menu.h"

using namespace pyrodactyl;

void OptionMenu::Init()
{
	XMLDoc layout_doc("core/data/ui_settings.xml");
	if (layout_doc.ready())
	{
		rapidxml::xml_node<char> *node = layout_doc.Doc()->first_node("settings");
		if (NodeValid(node))
		{
			if (NodeValid("category", node))
				category.Load(node->first_node("category"));

			if (NodeValid("audio", node))
				audio.Load(node->first_node("audio"));

			if (NodeValid("graphics", node))
				graphics.Load(node->first_node("graphics"));

			if (NodeValid("controller", node))
			{
				rapidxml::xml_node<char> *conode = node->first_node("controller");
				controller.Load(conode);

				if (NodeValid("img", conode))
					img_controller.Load(conode->first_node("img"));

				if (NodeValid("alt", conode))
					img_controller_alt.Load(conode->first_node("alt"));
			}

			if (NodeValid("resolution", node))
				me_resolution.Load(node->first_node("resolution"));

			if (NodeValid("keybind", node))
				me_keybind.Load(node->first_node("keybind"));
		}
	}

	//Initially set the slider value to volume value
	audio.menu.element.at(0).SliderValue(EnvValuef(ENV_VOLUME_MUSIC));
	audio.menu.element.at(1).SliderValue(EnvValuef(ENV_VOLUME_FX));
}

bool OptionMenu::HandleEvents(const bool esc_key_pressed, const bool game_running)
{
	switch (state)
	{
	case STATE_CATEGORY:
	{
		if (esc_key_pressed)
			return true;

		int choice = category.menu.HandleEvents();
		if (choice > -1)
		{
			switch (choice)
			{
			case 0: OptionsCheckAch(); state = STATE_AUDIO; break;
			case 1: OptionsCheckAch(); state = STATE_GRAPHICS; break;
			case 2: OptionsCheckAch(); state = STATE_KEYBIND; break;
			case 3: OptionsCheckAch(); state = STATE_CONTROLLER; break;
			case 4: return true;
			default: break;
			}
		}
	}
	break;
	case STATE_AUDIO:
	{
		if (esc_key_pressed)
			state = STATE_CATEGORY;

		int choice = audio.menu.HandleEvents();
		if (choice == 2)
			state = STATE_CATEGORY;

		EnvValueSetf(ENV_VOLUME_MUSIC, audio.menu.element.at(0).SliderValue());
		EnvValueSetf(ENV_VOLUME_FX, audio.menu.element.at(1).SliderValue());
	}
	break;
	case STATE_GRAPHICS:
	{
		graphics.menu.element.at(0).RadioState(EnvValueb(ENV_FULLSCREEN));
		graphics.menu.element.at(1).RadioState(!EnvValueb(ENV_NOTILT));
		graphics.menu.element.at(2).RadioState(EnvValueb(ENV_VSYNC));

		if (esc_key_pressed)
			state = STATE_CATEGORY;

		int choice = graphics.menu.HandleEvents();
		if (choice > -1)
		{
			switch (choice)
			{
			case 0:
				EnvValueSetb(ENV_FULLSCREEN, graphics.menu.element.at(0).RadioState());
				SystemSizeWindow ();
				break;
			case 1:
				EnvValueSetb(ENV_NOTILT, !graphics.menu.element.at(1).RadioState());
				break;
			case 2:
				EnvValueSetb(ENV_VSYNC, graphics.menu.element.at(2).RadioState());
				break;
			case 3: state = STATE_RESOLUTION; break;
			case 4: state = STATE_CATEGORY; break;
			default: break;
			}
		}
	}
	break;
	case STATE_RESOLUTION:
		if (me_resolution.HandleEvents(game_running) || esc_key_pressed)
			state = STATE_GRAPHICS;
		break;
	case STATE_KEYBIND:
		if (me_keybind.HandleEvents() || esc_key_pressed)
		{
			state = STATE_CATEGORY;
			gInput.Save();
		}
		break;
	case STATE_CONTROLLER:
	{
		controller.menu.element.at(0).RadioState(EnvValueb(ENV_CONTROLLER_ALT));

		int choice = controller.menu.HandleEvents();
		if (choice > -1)
		{
			switch (choice)
			{
			case 0:
				EnvValueSetb(ENV_CONTROLLER_ALT, controller.menu.element.at(0).RadioState());
				break;
			case 1: state = STATE_CATEGORY; break;
			default: break;
			}
		}

		if (esc_key_pressed)
			state = STATE_CATEGORY;
	}
	break;
	default: break;
	}

	return false;
}

void OptionMenu::Draw()
{
	switch (state)
	{
	case STATE_CATEGORY: category.Draw(); break;
	case STATE_AUDIO: audio.Draw(); break;
	case STATE_GRAPHICS: graphics.Draw(); break;
	case STATE_RESOLUTION: me_resolution.Draw(); break;
	case STATE_KEYBIND: me_keybind.Draw(); break;
	case STATE_CONTROLLER:
		controller.Draw();
		if (EnvValueb(ENV_CONTROLLER_ALT))
			img_controller_alt.Draw();
		else
			img_controller.Draw();

		break;
	default: break;
	}
}

void OptionMenu::SetUI()
{
	category.SetUI();
	audio.SetUI();
	graphics.SetUI();
	controller.SetUI();
	img_controller.SetUI();
	img_controller_alt.SetUI();

	me_resolution.SetUI();
	me_keybind.SetUI();
}