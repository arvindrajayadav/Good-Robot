#include "master.h"
#include "ui_upgrade.h"
#include "world.h"

using namespace pyrodactyl;

void UpgradeMenu::Init()
{
	XMLDoc layout_doc("core/data/ui_upgrade.xml");
	if (layout_doc.ready())
	{
		rapidxml::xml_node<char> *node = layout_doc.Doc()->first_node("upgrade");
		if (NodeValid(node))
		{
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			if (NodeValid("title", node))
				title.Load(node->first_node("title"));

			if (NodeValid("quit", node))
				quit.Load(node->first_node("quit"));

			if (NodeValid("money", node))
				money.Load(node->first_node("money"));

			money_sym = money.caption.text;

			if (NodeValid("template_button", node))
				t_bu.Load(node->first_node("template_button"));

			if (NodeValid("tooltip", node))
				tooltip.Load(node->first_node("tooltip"));

			if (NodeValid("menu", node))
			{
				rapidxml::xml_node<char> *menode = node->first_node("menu");

				//Only load menu type from XML
				upgrade.Load(menode);
				inc.Load(menode);
			}

			upgrade.Clear();

			if (NodeValid("skills", node))
			{
				int i = 0;
				for (rapidxml::xml_node<char> *n = node->first_node("skills")->first_node(); n != nullptr; n = n->next_sibling(), ++i)
				{
					UpgradeButton ub;
					ub.Init(t_bu, nullptr, inc.RawPosX() * (i / upgrade.Rows()), inc.RawPosY() * (i % upgrade.Rows()));
					LoadStr(ub.caption.text, "caption", n);
					LoadStr(ub.tooltip.text, "tooltip", n);
					upgrade.element.push_back(ub);
				}

				//This extra button is for upgrades
				UpgradeButton ub;
				ub.Init(t_bu, nullptr, inc.RawPosX() * (i / upgrade.Rows()), inc.RawPosY() * (i % upgrade.Rows()));
				upgrade.element.push_back(ub);
			}

			if (NodeValid("upgrades", node))
			{
				rapidxml::xml_node<char> *upnode = node->first_node("upgrades");
				LoadStr(ability.owned.caption, "owned_caption", upnode);
				LoadStr(ability.owned.tooltip, "owned_tooltip", upnode);

				int i = 0;
				for (rapidxml::xml_node<char> *n = upnode->first_node(); n != nullptr && i < ABILITY_TYPES; n = n->next_sibling(), ++i)
				{
					LoadStr(ability.data[i].caption, "caption", n);
					LoadStr(ability.data[i].tooltip, "tooltip", n);
					LoadNum(ability.data[i].cost, "cost", n);
				}
			}

			upgrade.AssignPaths();

			if (NodeValid("news", node))
				news.Load(node->first_node("news"));
		}
	}
}

bool UpgradeMenu::HandleEvents(const bool esc_key_pressed)
{
	PlayerStats *p = Player();

	int choice = upgrade.HandleEvents();
	if (choice > -1)
	{
		if (choice < SKILL_COUNT)
		{
			PlayerSkill s = static_cast<PlayerSkill>(choice);

			//Only buy if we are not at the maximum level of the skill
			if (p->SkillUpEligible(s))
			{
				int cur_level = p->Skill(s);
				int cost = EnvUpgradeCost(s, cur_level, p->UpgradeCount());

				//Do we have sufficient money to buy this?
				if (p->Money() >= cost)
				{
					p->SkillUp(s);
					p->MoneyGive(-1 * cost);

					//Refresh the menu - our stats have changed, and so have the costs
					PopulateMenu();
				}
			}
		}
		else
		{
			//Only buy if we don't own the ability already
			if (!p->Ability(ability.select) && ability.select > -1 && ability.select < ABILITY_TYPES)
			{
				int cost = ability.data[ability.select].cost;

				//Do we have sufficient money to buy this?
				if (p->Money() >= cost)
				{
					p->Ability(ability.select, true);
					p->MoneyGive(-1 * cost);

					//Change the menu to offer players a new upgrade
					PopulateMenu();
				}
			}
		}
	}

	int hover = upgrade.HoverIndex();
	if (hover > -1 && hover < upgrade.element.size())
	{
		tooltip.enabled = true;
		tooltip.text = upgrade.element.at(hover).tooltip.text;
	}
	else
		tooltip.enabled = false;

	if (news.HandleEvents())
		p->NewsCheckAch();

	return (quit.HandleEvents() == BUAC_LCLICK || esc_key_pressed);
}

void UpgradeMenu::Draw()
{
	PlayerStats *p = Player();

	if (refresh)
	{
		PopulateMenu();
		refresh = false;
	}

	bg.Draw();
	title.Draw();

	upgrade.Draw();
	tooltip.Draw();

	money.caption.text = money_sym + NumberToString(p->Money());
	money.Draw();

	if (last_world_loc_id != WorldLocationId())
	{
		//Get a new random news item
		news.Randomize(WorldLevelIndex());

		refresh = true;
		last_world_loc_id = WorldLocationId();
	}
	news.Draw();

	quit.Draw();
}

void UpgradeMenu::PopulateMenu()
{
	PlayerStats *p = Player();
	for (int i = 0; i < upgrade.element.size() && i < SKILL_COUNT; ++i)
	{
		PlayerSkill s = static_cast<PlayerSkill>(i);
		int cur_level = p->Skill(s), max_level = EnvSkillMaxLevel(s);
		int cost = EnvUpgradeCost(s, cur_level, p->UpgradeCount());
		upgrade.element.at(i).PopulateMenu(p->Skill(s), max_level, cost, p->Money());
	}

	//For the last button, choose a random ability not owned by the player and offer it for purchase
	std::vector<int> list;
	int pos = upgrade.element.size() - 1;

	for (int i = 0; i < ABILITY_TYPES; ++i)
		if (!p->Ability(i))
			list.push_back(i);

	//If the player owns all abilities, display a message saying "all power ups owned" or something
	if (list.empty())
	{
		ability.select = 0;

		//Set the caption and tooltip text
		upgrade.element.at(pos).caption.text = ability.owned.caption;
		upgrade.element.at(pos).tooltip.text = ability.owned.tooltip;
	}
	else
	{
		ability.select = list.at(RandomVal(list.size()));

		//Set the caption and tooltip text
		upgrade.element.at(pos).caption.text = ability.data[ability.select].caption;
		upgrade.element.at(pos).tooltip.text = ability.data[ability.select].tooltip;
	}

	upgrade.element.at(pos).Populate(!p->Ability(ability.select), ability.data[ability.select].cost, p->Money());
}

void UpgradeMenu::SetUI()
{
	bg.SetUI();
	title.SetUI();
	upgrade.SetUI();
	quit.SetUI();
	money.SetUI();
	tooltip.SetUI();
	news.SetUI();
}