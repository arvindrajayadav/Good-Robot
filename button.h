//=============================================================================
// Author:   Arvind
// Purpose:  Button class
//=============================================================================
#pragma once

#include "common_header.h"
#include "Caption.h"
#include "HoverInfo.h"
#include "hotkey.h"
#include "element.h"
#include "ButtonImage.h"
#include "RadioButtonData.h"
#include "SliderData.h"

namespace pyrodactyl
{
	class Button : public Element
	{
	protected:
		//The type of button
		enum { TYPE_SIMPLE, TYPE_RADIO, TYPE_SLIDER, TYPE_STATE } type;

		//Is this button activated?
		bool visible;

		//Used so we don't play the mouse hover sound over and over when the mouse is hovered
		bool hover_prev;

		//Can the player move this button?
		bool canmove;

		//The button images
		ButtonImage img;

		//A hotkey is a keyboard key(s) that are equivalent to pressing a button
		HotKey hotkey;

		//The radio button data
		RadioButtonData radio;

		//The slider data
		SliderData slider;

		//State button data
		struct
		{
			//We require two sets of images
			StateButtonImage img;

			//We also need two sets of colors
			struct StateButtonColor
			{
				int col, col_h, col_s;

				StateButtonColor() { col = 0; col_s = 0; col_h = 0; }
			} col_normal, col_select;
		} state_data;

		//Set the slider knob coordinates and slider value
		void SetSlider(const float val)
		{
			slider.Value(val);
			x = slider.x + slider.KnobPos(w);
			y = slider.y;
		}

		void IncSlider(const float inc)
		{
			SetSlider(slider.Value() + inc);
		}

	public:
		//Text shown all times on the button
		Caption caption, desc;

		//Text shown when mouse is hovered over the button
		HoverInfo tooltip;

		//We need to keep track of keyboard and mouse hovering separately
		bool hover_mouse, hover_key;

		//Has the mouse been pressed - but not released - over the button?
		//When user presses mouse inside the button, but then
		//moves mouse pointer away and releases it outside the button
		//UI practices in most OSes do not take this as a click event
		bool mousepressed;

		//Set the state of the button - false is original image, true is second image
		//Used only for state button
		void State(const bool val);

		//Set the state of the button - false is original image, true is second image
		//Used only for state button
		bool State() { return (img == state_data.img.select); }

		//Set the image - used only for state button
		void Img(const StateButtonImage &sbi);

		Button();
		~Button(){}
		void Reset();
		void SetUI(pyroRect *parent = nullptr);

		void Img(Button &b){ img = b.img; }
		void Img(ButtonImage &img){ this->img = img; }
		ButtonImage Img() { return img; }

		void Visible(const bool &val) { visible = val; }
		const bool Visible() { return visible; }

		const float SliderValue() { return slider.Value(); }
		void SliderValue(const float val) { slider.Value(val); }

		const bool RadioState() { return radio.state; }
		void RadioState(const bool val) { radio.state = val; }

		void ToggleRadioState()
		{
			if (type == TYPE_RADIO)
				radio.ToggleState();
		}

		void Load(rapidxml::xml_node<char> * node, pyroRect *parent = nullptr, const bool &echo = true);
		void Init(const Button &ref, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f);

		void Draw();
		ButtonAction HandleEvents();
	};
}