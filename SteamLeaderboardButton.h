#pragma once

#include "button.h"

namespace pyrodactyl
{
	class SteamLeaderboardButton : public Button
	{
	protected:
		//caption -> rank, desc -> player name, val -> the score itself
		//The rest are drawn using the structures below
		Caption val;

	public:
		void SetText(const int rank, const char* name, const int value)
		{
			caption.text = NumberToString(rank);
			desc.text = name;
			val.text = NumberToString(value);

			/*int min = (h.playtime / 1000) / 60;
			int sec = (h.playtime / 1000) % 60;
			playtime.text = NumberToString(min) + ":" + NumberToString(sec);*/
		}

		void Load(rapidxml::xml_node<char> * node, pyroRect *parent = nullptr, const bool &echo = true)
		{
			Button::Load(node, parent, echo);

			if (NodeValid("slider", node, false))
				val.Load(node->first_node("val"), &slider);
			else
				val.Load(node->first_node("val"), this);
		}

		void Init(const SteamLeaderboardButton &ref, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f)
		{
			Button::Init(ref, parent, XOffset, YOffset);
			val.Init(ref.val, this);
		}

		void Draw()
		{
			Button::Draw();
			if (visible)
			{
				if (mousepressed)
					val.Draw(CAP_SELECT);
				else if (hover_mouse || hover_key)
					val.Draw(CAP_HOVER);
				else
					val.Draw(CAP_NORMAL);
			}
		}

		void SetUI(pyroRect *parent = nullptr)
		{
			Button::SetUI(parent);
			val.SetUI(this);
		}
	};
}