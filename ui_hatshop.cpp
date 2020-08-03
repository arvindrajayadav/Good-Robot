#include "master.h"
#include "ui_hatshop.h"
#include "world.h"
#include "random.h"

using namespace pyrodactyl;

template<class bidiiter>
bidiiter random_unique(bidiiter begin, bidiiter end, size_t num_random) {
	size_t left = std::distance(begin, end);
	while (num_random--) {
		bidiiter r = begin;
		std::advance(r, RandomVal() % left);
		std::swap(*begin, *r);
		++begin;
		--left;
	}
	return begin;
}

void HatShopMenu::Init()
{
	XMLDoc layout_doc("core/data/ui_hat.xml");
	if (layout_doc.ready())
	{
		rapidxml::xml_node<char> *node = layout_doc.Doc()->first_node("store");
		if (NodeValid(node))
		{
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			if (NodeValid("title", node))
				title.Load(node->first_node("title"));

			if (NodeValid("template_button", node))
				t_bu.Load(node->first_node("template_button"));

			if (NodeValid("menu", node))
			{
				rapidxml::xml_node<char> *menode = node->first_node("menu");

				//Only load menu type from XML
				menu.Load(menode);
				inc.Load(menode);
			}

			for (int i = 0; i < STORE_HAT_SLOTS; ++i)
			{
				StoreButton sb;
				sb.Init(t_bu, nullptr, inc.RawPosX() * (i / menu.Rows()), inc.RawPosY() * (i % menu.Rows()));
				menu.element.push_back(sb);
			}

			if (NodeValid("menu", node))
				menu.Load(node->first_node("menu"));

			if (NodeValid("quit", node))
				quit.Load(node->first_node("quit"));

			if (NodeValid("money", node))
				money.Load(node->first_node("money"));

			if (NodeValid("tutorial", node))
				tutorial.Load(node->first_node("tutorial"));

			if (NodeValid("tooltip", node))
				tooltip.Load(node->first_node("tooltip"));

			if (NodeValid("hat_purchased_message", node))
				hat_purchased_message.Load(node->first_node("hat_purchased_message"));

			if (NodeValid("hats", node))
			{
				rapidxml::xml_node<char> *hatnode = node->first_node("hats");

				LoadStr(text_first, "text_first", hatnode);

				hat_collection.clear();
				for (rapidxml::xml_node<char> *n = hatnode->first_node(); n != nullptr; n = n->next_sibling())
				{
					HatInfo h;
					h.Load(n);
					h.id = hat_collection.size();

					hat_collection.push_back(h);
				}
			}
		}
	}

	money_sym = money.caption.text;
}

void HatShopMenu::Draw()
{
	PlayerStats *p = Player();
	bg.Draw();
	title.Draw();

	//Don't draw the store if the player has purchased a hat in this zone
	if (p->HatPurchasedThisZone())
		hat_purchased_message.Draw();
	else
	{
		if (p->HatLost())
		{
			refresh = true;
			p->HatLost(false);
		}

		if (refresh)
		{
			Refresh(p);
			refresh = false;
		}

		menu.Draw();
		tooltip.Draw();
	}

	tutorial.Draw();

	money.caption.text = money_sym + NumberToString(p->Money());
	money.Draw();

	quit.Draw();
}

bool HatShopMenu::HandleEvents(const bool esc_key_pressed)
{
	PlayerStats *p = Player();

	//Only allow purchasing stuff if you've not bought a hat in this zone
	if (!p->HatPurchasedThisZone())
	{
		int cost = 0;

		//Used to decide which tooltip to display
		int choice = menu.HoverIndex();
		tooltip.enabled = (choice != -1);

		if (p->HatPurchaseCount() == 0)
			tooltip.text = text_first;
		else if (choice >= 0 && choice < STORE_HAT_SLOTS)
			tooltip.text = HatFromID(id_hat[choice])->desc;

		//If the user buys a hat
		choice = menu.HandleEvents();
		if (choice >= 0 && choice < STORE_HAT_SLOTS)
		{
			HatInfo *h = HatFromID(id_hat[choice]);

			//We want to buy a hat
			if (p->HatPurchaseCount() == 0)
				cost = 0;
			else
				cost = h->cost;

			if (p->Money() >= cost)
			{
				//Deduct cost of hat
				p->MoneyGive(-1 * cost);

				//Give player the primary weapon
				PlayerHatGive(h->name, h->sprite, h->size, h->color);
				p->HatPurchaseCountInc();
				p->HatCheckAch();
				refresh = true;
				p->HatPurchasedThisZone(true);

				//We exit the hat store as soon as we buy a hat
				return true;
			}
		}
	}

	if (tutorial.HandleEvents())
		p->TutorialCheckAch();

	return (quit.HandleEvents() == BUAC_LCLICK || esc_key_pressed);
}

void HatShopMenu::Refresh(PlayerStats *p)
{
	for (int i = 0; i < menu.element.size() && i < STORE_HAT_SLOTS; ++i)
	{
		HatInfo* h = HatFromID(id_hat[i]);

		menu.element.at(i).caption.text = h->name;
		menu.element.at(i).tooltip.text = h->desc;

		if (p->HatPurchaseCount() == 0)
			menu.element.at(i).SetCostText(p->Hat() && p->HatName() == h->name, 0, p->Money(), SpriteEntryLookup(h->sprite), h->color);
		else
			menu.element.at(i).SetCostText(p->Hat() && p->HatName() == h->name, h->cost, p->Money(), SpriteEntryLookup(h->sprite), h->color);
	}
}

void HatShopMenu::Randomize()
{
	refresh = true;
	if (last_world_loc_id != WorldLocationId())
	{
		//Get random hats for all slots
		std::vector<HatInfo> set = hat_collection;
		random_unique(set.begin(), set.end(), STORE_HAT_SLOTS);

		for (int i = 0; i < set.size() && i < STORE_HAT_SLOTS; ++i)
			id_hat[i] = set.at(i).id;

		Player()->HatPurchasedThisZone(false);
		last_world_loc_id = WorldLocationId();
	}
}

void HatShopMenu::SetUI()
{
	bg.SetUI();
	title.SetUI();
	menu.SetUI();
	quit.SetUI();
	money.SetUI();
	tooltip.SetUI();
	tutorial.SetUI();
	hat_purchased_message.SetUI();
}