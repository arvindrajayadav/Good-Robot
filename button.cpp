//=============================================================================
// Author:   Arvind
// Purpose:  Contains the button functions
//=============================================================================
#include "master.h"
#include "button.h"
#include "audio.h"

using namespace pyrodactyl;

//------------------------------------------------------------------------
// Purpose: Constructor
//------------------------------------------------------------------------
Button::Button()
{
	visible = false;
	canmove = false;
	hover_prev = false;
	type = TYPE_SIMPLE;
	Reset();
}
//------------------------------------------------------------------------
// Purpose: Load a new Button from a file
//------------------------------------------------------------------------
void Button::Load(rapidxml::xml_node<char> * node, pyroRect *parent, const bool &echo)
{
	img.Load(node, echo);

	if (NodeValid("slider", node, false))
	{
		type = TYPE_SLIDER;
		canmove = true;
		slider.Load(node->first_node("slider"), parent);
		Element::Load(node, &slider, echo);

		//Since this is a slider, the button position (x,y) doesn't matter at all
		//It's set based on the value of the slider
		slider.Value(0.5f);

		x = slider.x + slider.Offset(w);
		y = slider.y;

		//In case of a slider, the knob moves around - so the caption and tooltip
		//positions are relative to the background image
		tooltip.Load(node->first_node("tooltip"), &slider);
		caption.Load(node->first_node("caption"), &slider);
	}
	else
	{
		Element::Load(node, parent, echo);

		if (NodeValid("radio", node, false))
		{
			type = TYPE_RADIO;
			radio.Load(node->first_node("radio"), this);
		}
		else if (NodeValid("select", node, false))
		{
			type = TYPE_STATE;
			rapidxml::xml_node<char> *selnode = node->first_node("select");

			state_data.img.select.Load(selnode, echo);
			LoadNum(state_data.col_select.col, "color", selnode);
			LoadNum(state_data.col_select.col_s, "color_s", selnode);
			LoadNum(state_data.col_select.col_h, "color_h", selnode);

			state_data.img.normal = img;
			state_data.col_normal.col = caption.col;
			state_data.col_normal.col_s = caption.col_s;
			state_data.col_normal.col_h = caption.col_h;
		}
		else
			type = TYPE_SIMPLE;

		tooltip.Load(node->first_node("tooltip"), this);
		caption.Load(node->first_node("caption"), this);
		desc.Load(node->first_node("desc"), this);
	}

	if (NodeValid("hotkey", node, false))
		hotkey.Load(node->first_node("hotkey"), this);

	visible = true;
	canmove = false;
	Reset();
}
//------------------------------------------------------------------------
// Purpose: Load a new Button
//------------------------------------------------------------------------
void Button::Init(const Button &ref, pyroRect *parent, const float &XOffset, const float &YOffset)
{
	img = ref.img;
	type = ref.type;
	Element::Init(ref, parent, XOffset, YOffset);

	caption.Init(ref.caption, this);
	tooltip.Init(ref.tooltip, this);
	desc.Init(ref.desc, this);

	state_data = ref.state_data;

	visible = true;
	canmove = ref.canmove;
	Reset();
}
//------------------------------------------------------------------------
// Purpose: Reset the button
//------------------------------------------------------------------------
void Button::Reset()
{
	mousepressed = false;
	hover_mouse = false;
	hover_key = false;
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Button::Draw()
{
	if (visible)
	{
		if (type == TYPE_SLIDER)
			slider.Draw();

		if (mousepressed)
		{
			gImageManager.Draw(x, y, w, h, img.select);

			tooltip.Draw();
			caption.Draw(CAP_SELECT);
			desc.Draw(CAP_SELECT);
		}
		else if (hover_mouse || hover_key)
		{
			gImageManager.Draw(x, y, w, h, img.hover);

			tooltip.Draw();
			caption.Draw(CAP_HOVER);
			desc.Draw(CAP_HOVER);
		}
		else
		{
			gImageManager.Draw(x, y, w, h, img.normal);
			caption.Draw(CAP_NORMAL);
			desc.Draw(CAP_NORMAL);
		}

		if (type == TYPE_RADIO)
			radio.Draw(x, y);

		hotkey.Draw();
	}
}
//------------------------------------------------------------------------
// Purpose: Handle input and stuff
//------------------------------------------------------------------------
ButtonAction Button::HandleEvents()
{
	pyroRect dim = *this;

	if (visible)
	{
		GLcoord2 pos = InputMousePosition();

		if (InputKeyState(INPUT_MOUSE_MOVED))
		{
			if (dim.Contains(pos.x, pos.y))
			{
				hover_mouse = true;

				if (!hover_prev)
				{
					hover_prev = true;
					AudioPlay("mouseover");
				}
			}
			else
			{
				hover_prev = false;
				hover_mouse = false;
			}

			if (canmove && mousepressed)
			{
				GLcoord2 motion = InputMouseMovement();

				if (type == TYPE_SLIDER)
				{
					x += motion.x;
					if (x < slider.x)
						x = slider.x;
					else if (x + w > slider.x + slider.w)
						x = slider.x + slider.w - w;

					y = slider.y;
					slider.Value(x, w);
				}
				else
				{
					x += motion.x;
					y += motion.y;
				}

				return BUAC_GRABBED;
			}
		}
		else if (InputKeyState(INPUT_LMB))
		{
			//Mouse button pressed, then released, comprises of a click action
			if (dim.Contains(pos.x, pos.y))
				mousepressed = true;
			else if (type == TYPE_SLIDER) //If a person clicks on the slider bar, the knob needs to travel there
			{
				if (slider.Contains(pos.x, pos.y)) //incorporate  if we need it
				{
					x = pos.x;
					y = slider.y;
					slider.Value(x, w);

					mousepressed = true;
					return BUAC_GRABBED;
				}
			}
		}
		else if (!InputKeyState(INPUT_LMB) && mousepressed)
		{
			Reset();
			if (dim.Contains(pos.x, pos.y))
			{
				mousepressed = false;
				AudioPlay("click");
				ToggleRadioState();

				return BUAC_LCLICK;
			}
		}
		else if (hotkey.HandleEvents())
		{
			AudioPlay("click");
			ToggleRadioState();

			return BUAC_LCLICK;
		}

		if (type == TYPE_SLIDER && (hover_key || hover_mouse))
		{
			if (gInput.Pressed(CONTROL_RIGHT))     { IncSlider(slider.Inc()); }
			else if (gInput.Pressed(CONTROL_LEFT)) { IncSlider(-1 * slider.Inc()); }
		}
	}

	return BUAC_IGNORE;
}
//------------------------------------------------------------------------
// Purpose: State button functions
//------------------------------------------------------------------------
void Button::State(const bool val)
{
	if (val)
	{
		img = state_data.img.select;
		caption.col = state_data.col_select.col;
		caption.col_s = state_data.col_select.col_s;
		caption.col_h = state_data.col_select.col_h;
	}
	else
	{
		img = state_data.img.normal;
		caption.col = state_data.col_normal.col;
		caption.col_s = state_data.col_normal.col_s;
		caption.col_h = state_data.col_normal.col_h;
	}

	//Images might be different in size
	//w = gImageManager.GetTexture(img.normal)->Size().x;
	//h = gImageManager.GetTexture(img.normal)->Size().y;
}

void Button::Img(const StateButtonImage &sbi)
{
	//Find which is the current image and set it
	if (img == state_data.img.normal)
		img = sbi.normal;
	else
		img = sbi.select;

	state_data.img = sbi;
}
//------------------------------------------------------------------------
// Purpose: Set UI position
//------------------------------------------------------------------------
void Button::SetUI(pyroRect *parent)
{
	Element::SetUI(parent);

	if (type == TYPE_SLIDER)
	{
		slider.SetUI();

		//The position (x,y) of the knob doesn't matter
		//It depends on the value and position of the background image
		x = slider.x + slider.KnobPos(w);
		y = slider.y;

		//In case of a slider, the knob moves around - so the caption and tooltip
		//positions are relative to the background image
		tooltip.SetUI(&slider);
		caption.SetUI(&slider);
	}
	else
	{
		if (type == TYPE_RADIO)
			radio.SetUI(this);

		tooltip.SetUI(this);
		caption.SetUI(this);
		desc.SetUI(this);
		hotkey.SetUI(this);
	}
}