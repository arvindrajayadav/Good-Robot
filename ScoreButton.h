#pragma once

#include "button.h"
#include "HighScoreData.h"

namespace pyrodactyl
{
	class ScoreButton : public Button
	{
	protected:
		//caption -> name, desc -> score
		//The rest are drawn using the structures below
		Caption kills, money, playtime, upgrade, warranty;

		//Only draw the text if the entry is valid
		bool valid;

	public:
		void SetText(const HighScoreData &h)
		{
			valid = h.valid;

			if (valid)
			{
				caption.text = h.name;
				desc.text = NumberToString(h.score);
				kills.text = NumberToString(h.kills);
				money.text = NumberToString(h.money_earned);
				upgrade.text = NumberToString(h.upgrade_count);
				warranty.text = NumberToString(h.warranty_count);

				int min = (h.playtime / 1000) / 60;
				int sec = (h.playtime / 1000) % 60;
				playtime.text = NumberToString(min) + ":" + NumberToString(sec);
			}
		}

		void TrimName() { caption.text.resize(10); }
		void Valid(bool val) { valid = val; }

		void Load(rapidxml::xml_node<char> * node, pyroRect *parent = nullptr, const bool &echo = true)
		{
			Button::Load(node, parent, echo);

			if (NodeValid("slider", node, false))
			{
				kills.Load(node->first_node("kills"), &slider);
				money.Load(node->first_node("money"), &slider);
				playtime.Load(node->first_node("time"), &slider);
				upgrade.Load(node->first_node("upgrade"), &slider);
				warranty.Load(node->first_node("warranty"), &slider);
			}
			else
			{
				kills.Load(node->first_node("kills"), this);
				money.Load(node->first_node("money"), this);
				playtime.Load(node->first_node("time"), this);
				upgrade.Load(node->first_node("upgrade"), this);
				warranty.Load(node->first_node("warranty"), this);
			}
		}

		void Init(const ScoreButton &ref, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f)
		{
			Button::Init(ref, parent, XOffset, YOffset);

			kills.Init(ref.kills, this);
			money.Init(ref.money, this);
			playtime.Init(ref.playtime, this);
			upgrade.Init(ref.upgrade, this);
			warranty.Init(ref.warranty, this);
		}

		void Draw()
		{
			if (valid)
			{
				Button::Draw();
				if (visible)
				{
					if (mousepressed)
					{
						kills.Draw(CAP_SELECT);
						money.Draw(CAP_SELECT);
						playtime.Draw(CAP_SELECT);
						upgrade.Draw(CAP_SELECT);
						warranty.Draw(CAP_SELECT);
					}
					else if (hover_mouse || hover_key)
					{
						kills.Draw(CAP_HOVER);
						money.Draw(CAP_HOVER);
						playtime.Draw(CAP_HOVER);
						upgrade.Draw(CAP_HOVER);
						warranty.Draw(CAP_HOVER);
					}
					else
					{
						kills.Draw(CAP_NORMAL);
						money.Draw(CAP_NORMAL);
						playtime.Draw(CAP_NORMAL);
						upgrade.Draw(CAP_NORMAL);
						warranty.Draw(CAP_NORMAL);
					}
				}
			}
		}

		void SetUI(pyroRect *parent = nullptr)
		{
			Button::SetUI(parent);
			kills.SetUI(this);
			money.SetUI(this);
			playtime.SetUI(this);
			upgrade.SetUI(this);
			warranty.SetUI(this);
		}
	};
}