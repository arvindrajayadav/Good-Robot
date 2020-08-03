#include "master.h"
#include "ui_store.h"
#include "projectile.h"
#include "world.h"
#include "fx.h"
#include "entity.h"

using namespace pyrodactyl;

StoreMenu::StoreMenu()
{
	Reset(); prev_shields = 0;

	for (int i = 0; i < STORE_WEAPON_SLOTS; ++i)
	{
		id_prim[i] = 0;
		id_sec[i] = 0;
	}
}

void StoreMenu::Init()
{
	XMLDoc layout_doc("core/data/ui_store.xml");
	if (layout_doc.ready())
	{
		rapidxml::xml_node<char> *node = layout_doc.Doc()->first_node("store");
		if (NodeValid(node))
		{
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			if (NodeValid("title", node))
				title.Load(node->first_node("title"));

			if (NodeValid("menu", node))
				menu.Load(node->first_node("menu"));

			if (NodeValid("quit", node))
				quit.Load(node->first_node("quit"));

			if (NodeValid("money", node))
				money.Load(node->first_node("money"));

			if (NodeValid("tooltip", node))
				tooltip.Load(node->first_node("tooltip"));

			if (NodeValid("news", node))
				news.Load(node->first_node("news"));

			if (NodeValid("desc", node))
			{
				rapidxml::xml_node<char> *desnode = node->first_node("desc");
				LoadStr(desc.warranty, "warranty", desnode);
				LoadStr(desc.repair, "repair", desnode);
			}
		}
	}

	LoadNewsText();
	money_sym = money.caption.text;
}

void StoreMenu::SpawnWeapon(int weapon_id)
{
	fxPickup*   p = new fxPickup();
	p->InitGun(WorldItemDropoff(), weapon_id);
	EntityFxAdd(p);
}

void StoreMenu::Draw()
{
	PlayerStats *p = Player();

	bg.Draw();
	title.Draw();

	if (prev_shields != p->Shields())
	{
		refresh = true;
		prev_shields = p->Shields();
	}

	if (refresh)
	{
		Refresh(p);
		refresh = false;
	}

	menu.Draw();
	tooltip.Draw();
	news.Draw();

	money.caption.text = money_sym + StringNumberFormat(p->Money());
	money.Draw();

	quit.Draw();
}

bool StoreMenu::HandleEvents(const bool esc_key_pressed)
{
	PlayerStats *p = Player();

	int cost = 0;

	//Used to decide which tooltip to display
	int choice = menu.HoverIndex();
	tooltip.enabled = (choice != -1);
	switch (choice)
	{
	case 0: tooltip.text = desc.warranty; break;
	case 1: tooltip.text = desc.repair; break;
	case 2: tooltip.text = EnvProjectileFromId(id_prim[0])->_description; break;
	case 3: tooltip.text = EnvProjectileFromId(id_prim[1])->_description; break;
	case 4: tooltip.text = EnvProjectileFromId(id_sec[0])->_description; break;
	case 5: tooltip.text = EnvProjectileFromId(id_sec[1])->_description; break;
	default: break;
	}

	//If the user buys anything
	choice = menu.HandleEvents();
	switch (choice)
	{
	case 0:
		//Only buy warranty if the player doesn't have it
		if (!p->Warranty())
		{
			//We want to buy the warranty
			cost = EnvWarrantyCost(p->UpgradeCount(), p->WarrantyCount());
			if (p->Money() >= cost)
			{
				//Deduct cost of warranty
				p->MoneyGive(-1 * cost);

				//Give warranty and increase warranty purchased count by 1
				p->WarrantySet(true);
				p->WarrantyCountInc();
				p->WarrantyCheckAch();
				refresh = true;
			}
		}
		break;
	case 1:
		//Only buy health if the player is damaged
		if (!p->ShieldsFull())
		{
			//We want to buy health
			cost = EnvRepairCost(1.0f - p->ShieldsPercent(), p->RepairCount());
			if (p->Money() >= cost)
			{
				//Deduct cost of health
				p->MoneyGive(-1 * cost);

				//Give full shields and increase repair count by 1
				p->RestoreShields();
				p->RepairCountInc();
				p->RepairCheckAch();
				refresh = true;
			}
		}
		break;

	case 2:
		//Only buy the weapon if our weapon is not the same
		if (p->Weapon(PLAYER_WEAPON_PRIMARY)->Info()->_id != id_prim[0])
		{
			//We want to buy primary weapon
			cost = EnvProjectileFromId(id_prim[0])->_cost;
			if (p->Money() >= cost)
			{
				//Deduct cost of weapon
				p->MoneyGive(-1 * cost);

				//Spawn current weapon in the level so the player can pick it up again if they want
				SpawnWeapon(p->Weapon(PLAYER_WEAPON_PRIMARY)->Info()->_id);

				//Give player the primary weapon
				p->Weapon(PLAYER_WEAPON_PRIMARY)->Equip(EnvProjectileFromId(id_prim[0]));
				p->WeaponPurchaseCountInc();
				p->WeaponCheckAch();
				refresh = true;
			}
		}
		break;

	case 3:
		//Only buy the weapon if our weapon is not the same
		if (p->Weapon(PLAYER_WEAPON_PRIMARY)->Info()->_id != id_prim[1])
		{
			//We want to buy primary weapon
			cost = EnvProjectileFromId(id_prim[1])->_cost;
			if (p->Money() >= cost)
			{
				//Deduct cost of weapon
				p->MoneyGive(-1 * cost);

				//Spawn current weapon in the level so the player can pick it up again if they want
				SpawnWeapon(p->Weapon(PLAYER_WEAPON_PRIMARY)->Info()->_id);

				//Give player the primary weapon
				p->Weapon(PLAYER_WEAPON_PRIMARY)->Equip(EnvProjectileFromId(id_prim[1]));
				p->WeaponPurchaseCountInc();
				p->WeaponCheckAch();
				refresh = true;
			}
		}
		break;

	case 4:
		//Only buy the weapon if our weapon is not the same
		if (p->Weapon(PLAYER_WEAPON_SECONDARY)->Info()->_id != id_sec[0])
		{
			//We want to buy secondary weapon
			cost = EnvProjectileFromId(id_sec[0])->_cost;
			if (p->Money() >= cost)
			{
				//Deduct cost of weapon
				p->MoneyGive(-1 * cost);

				//Spawn current weapon in the level so the player can pick it up again if they want
				SpawnWeapon(p->Weapon(PLAYER_WEAPON_SECONDARY)->Info()->_id);

				//Give player the secondary weapon
				p->Weapon(PLAYER_WEAPON_SECONDARY)->Equip(EnvProjectileFromId(id_sec[0]));
				p->WeaponPurchaseCountInc();
				p->WeaponCheckAch();
				refresh = true;
			}
		}
		break;

	case 5:
		//Only buy the weapon if our weapon is not the same
		if (p->Weapon(PLAYER_WEAPON_SECONDARY)->Info()->_id != id_sec[1])
		{
			//We want to buy secondary weapon
			cost = EnvProjectileFromId(id_sec[1])->_cost;
			if (p->Money() >= cost)
			{
				//Deduct cost of weapon
				p->MoneyGive(-1 * cost);

				//Spawn current weapon in the level so the player can pick it up again if they want
				SpawnWeapon(p->Weapon(PLAYER_WEAPON_SECONDARY)->Info()->_id);

				//Give player the secondary weapon
				p->Weapon(PLAYER_WEAPON_SECONDARY)->Equip(EnvProjectileFromId(id_sec[1]));
				p->WeaponPurchaseCountInc();
				p->WeaponCheckAch();
				refresh = true;
			}
		}
		break;

	default: break;
	}

	if (news.HandleEvents())
		p->NewsCheckAch();

	return (quit.HandleEvents() == BUAC_LCLICK || esc_key_pressed);
}

void StoreMenu::Refresh(PlayerStats *p)
{
	//Our menu must have 6 buttons minimum
	if (menu.element.size() >= 6)
	{
		//If we have a warranty, hide the "buy" button for warranty and show the description
		int cost = EnvWarrantyCost(p->UpgradeCount(), p->WarrantyCount());
		menu.element.at(0).SetCostText(p->Warranty(), cost, p->Money());

		//If our shields are full, hide the "buy" button for repairs and show default text
		cost = EnvRepairCost(1.0f - p->ShieldsPercent(), p->RepairCount());
		menu.element.at(1).SetCostText(p->ShieldsFull(), cost, p->Money());

		//Set button description from projectile name
		//If we have the same weapon, don't show the buy button and show the default text for the button

		const Projectile *p0 = EnvProjectileFromId(id_prim[0]);
		menu.element.at(2).desc.text = p0->_title;
		menu.element.at(2).SetCostText(p->Weapon(PLAYER_WEAPON_PRIMARY)->Info()->_name == p0->_name, p0->_cost, p->Money(), p0->_icon, p0->_color);

		const Projectile *p1 = EnvProjectileFromId(id_prim[1]);
		menu.element.at(3).desc.text = p1->_title;
		menu.element.at(3).SetCostText(p->Weapon(PLAYER_WEAPON_PRIMARY)->Info()->_name == p1->_name, p1->_cost, p->Money(), p1->_icon, p1->_color);

		const Projectile *s0 = EnvProjectileFromId(id_sec[0]);
		menu.element.at(4).desc.text = s0->_title;
		menu.element.at(4).SetCostText(p->Weapon(PLAYER_WEAPON_SECONDARY)->Info()->_name == s0->_name, s0->_cost, p->Money(), s0->_icon, s0->_color);

		const Projectile *s1 = EnvProjectileFromId(id_sec[1]);
		menu.element.at(5).desc.text = s1->_title;
		menu.element.at(5).SetCostText(p->Weapon(PLAYER_WEAPON_SECONDARY)->Info()->_name == s1->_name, s1->_cost, p->Money(), s1->_icon, s1->_color);
	}
}

void StoreMenu::Randomize()
{
	refresh = true;
	if (last_world_loc_id != WorldLocationId())
	{
		//Get random weapons for each of the primary and secondary categories
		EnvProjectileRandomShop(PROJECTILE_PRIMARY, Player()->Weapon(PLAYER_WEAPON_PRIMARY)->Info(), WorldLevelIndex(), id_prim[0], id_prim[1]);

		//Do all of the above stuff for the secondary weapon
		EnvProjectileRandomShop(PROJECTILE_SECONDARY, Player()->Weapon(PLAYER_WEAPON_SECONDARY)->Info(), WorldLevelIndex(), id_sec[0], id_sec[1]);

		//Get a new random news item
		news.Randomize(WorldLevelIndex());

		last_world_loc_id = WorldLocationId();
	}
}

void StoreMenu::SetUI()
{
	bg.SetUI();
	title.SetUI();
	menu.SetUI();
	quit.SetUI();
	money.SetUI();
	tooltip.SetUI();
	news.SetUI();
}