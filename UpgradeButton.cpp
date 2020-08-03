#include "master.h"
#include "UpgradeButton.h"

using namespace pyrodactyl;

void UpgradeButton::Load(rapidxml::xml_node<char> * node, pyroRect *parent, const bool &echo)
{
	Button::Load(node, parent, echo);

	if (NodeValid("buy", node))
	{
		rapidxml::xml_node<char> *buynode = node->first_node("buy");
		buy.Load(buynode, this);

		if (NodeValid("price", buynode))
			price.Load(buynode->first_node("price"), &buy);
	}

	if (NodeValid("skill_points", node))
	{
		rapidxml::xml_node<char> *ptnode = node->first_node("skill_points");
		skill_ref.Load(ptnode, this);

		if (NodeValid("inc", ptnode))
			pts_inc.Load(ptnode->first_node("inc"));
	}
}

void UpgradeButton::Init(const UpgradeButton &ref, pyroRect *parent, const float &XOffset, const float &YOffset)
{
	Button::Init(ref, parent, XOffset, YOffset);
	buy.Init(ref.buy, this);
	skill_ref.Init(ref.skill_ref, this);
	pts_inc.Init(ref.pts_inc);
	price.Init(ref.price, &buy);
}

ButtonAction UpgradeButton::HandleEvents()
{
	me_skill.HandleEvents();
	Button::HandleEvents();
	return buy.HandleEvents();
}

void UpgradeButton::Draw()
{
	Button::Draw();
	buy.Draw();
	price.Draw();
	me_skill.Draw();
}

void UpgradeButton::PopulateMenu(const int &cur_points, const int &max_points, const int &cost, const long player_money)
{
	if (cur_points < max_points)
		price.text = NumberToString(cost);
	else
		price.text = "";

	buy.Visible(cur_points < max_points && player_money >= cost);

	me_skill.Clear();

	if (max_points > 1)
	{
		//Multiple buttons: 0 to cur_points-1 are selected, then cur_points to max_points are normal buttons

		//The first button is always start
		{
			Button sb;
			sb.Init(skill_ref.start, this);
			sb.State(cur_points > 0); //The state depends on if there is a single point in here
			me_skill.element.push_back(sb);
		}

		//All buttons except the last are mid
		for (int i = 1; i < max_points - 1; ++i)
		{
			Button sb;
			sb.Init(skill_ref.mid, this, i*pts_inc.RawPosX(), i*pts_inc.RawPosY());
			sb.State(i < cur_points); //Select the number of buttons equal to cur_points
			me_skill.element.push_back(sb);
		}

		//Last button is always end
		{
			Button sb;
			sb.Init(skill_ref.end, this, (max_points - 1) * pts_inc.RawPosX(), (max_points - 1) * pts_inc.RawPosY());
			sb.State(cur_points >= max_points); //Selected if we've spent all our points
			me_skill.element.push_back(sb);
		}
	}
	else
	{
		//Add a single button
		Button sb;
		sb.Init(skill_ref.single, this);

		//If no points put into here, state is normal
		//If this has a point inside it, state is selected
		sb.State(cur_points == max_points);

		me_skill.element.push_back(sb);
	}
}

void UpgradeButton::Populate(const bool &val, const int &cost, const long player_money)
{
	if (val)
		price.text = NumberToString(cost);
	else
		price.text = "";

	buy.Visible(player_money >= cost && val);
	me_skill.Clear();
}

void UpgradeButton::SetUI(pyroRect *parent)
{
	Button::SetUI(parent);
	buy.SetUI(this);
	skill_ref.SetUI(this);
	pts_inc.SetUI();
	me_skill.SetUI(this);
	price.SetUI(&buy);
}