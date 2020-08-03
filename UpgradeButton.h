#pragma once

#include "button.h"
#include "ui_menu.h"

namespace pyrodactyl
{
	//This is a button used to house one branch of the upgrade menu
	//The button itself is the outline image, the caption is the name of the skill tree and the tooltip is the description
	class UpgradeButton : public Button
	{
		//The set of buttons used to populate the menu that show how many points were put into this skill
		struct UpgradeButtonReference
		{
			Button single, start, mid, end;

			void Load(rapidxml::xml_node<char> *node, pyroRect* parent = nullptr, const bool &echo = true)
			{
				if (NodeValid(node, "single"), echo)
					single.Load(node->first_node("single"), parent, echo);

				if (NodeValid(node, "start"), echo)
					start.Load(node->first_node("start"), parent, echo);

				if (NodeValid(node, "mid"), echo)
					mid.Load(node->first_node("mid"), parent, echo);

				if (NodeValid(node, "end"), echo)
					end.Load(node->first_node("end"), parent, echo);
			}

			void Init(const UpgradeButtonReference &ref, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f)
			{
				single.Init(ref.single, parent, XOffset, YOffset);
				start.Init(ref.start, parent, XOffset, YOffset);
				mid.Init(ref.mid, parent, XOffset, YOffset);
				end.Init(ref.end, parent, XOffset, YOffset);
			}

			void SetUI(pyroRect *parent = nullptr)
			{
				single.SetUI(parent);
				start.SetUI(parent);
				mid.SetUI(parent);
				end.SetUI(parent);
			}
		} skill_ref;

		//The menu that stores the sequence of skill tree buttons
		Menu<Button> me_skill;

		//The offsets and increments for drawing the points
		Element pts_inc;

		//The buy button
		Button buy;

		//We need to draw the cost as a separate object, because otherwise setting buy button to invisible also
		//stops the cost from being drawn
		Caption price;

	public:
		UpgradeButton(){}
		~UpgradeButton(){}

		//For skills with multiple tiers
		void PopulateMenu(const int &cur_points, const int &max_points, const int &cost, const long player_money);

		//For abilities with true/false
		void Populate(const bool &val, const int &cost, const long player_money);

		void Load(rapidxml::xml_node<char> * node, pyroRect *parent = nullptr, const bool &echo = true);
		void Init(const UpgradeButton &ref, pyroRect *parent = nullptr, const float &XOffset = 0.0f, const float &YOffset = 0.0f);

		ButtonAction HandleEvents();
		void Draw();

		void SetUI(pyroRect *parent = nullptr);
	};
}