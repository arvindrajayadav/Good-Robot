#include "master.h"
#include "ui_hud.h"
#include "world.h"

using namespace pyrodactyl;

void HUD::Load(const std::string &filename)
{
	XMLDoc layout_doc(filename);
	if (layout_doc.ready())
	{
		rapidxml::xml_node<char> *node = layout_doc.Doc()->first_node("hud");
		if (NodeValid(node))
		{
			if (NodeValid("health", node))
				health_bar.Load(node->first_node("health"));

			if (NodeValid("boss", node))
				boss_bar.Load(node->first_node("boss"));

			if (NodeValid("world_title", node))
				world_title.Load(node->first_node("world_title"));

			if (NodeValid("zone_clear", node))
				zone_clear.Load(node->first_node("zone_clear"));

			if (NodeValid("score", node))
			{
				rapidxml::xml_node<char> *scnode = node->first_node("score");
				score.Load(scnode);
				LoadNum(limit, "limit", scnode);
			}

			if (NodeValid("money", node))
				money.Load(node->first_node("money"));

			if (NodeValid("warranty", node))
				warranty.Load(node->first_node("warranty"));

			if (NodeValid("primary", node))
				weapon_primary.Load(node->first_node("primary"));

			if (NodeValid("secondary", node))
				weapon_secondary.Load(node->first_node("secondary"));

			if (NodeValid("compass", node))
				compass.Load(node->first_node("compass"));
		}

		//Initialize once so its actually drawn
		PlayerStats *p = Player();
		score.caption.text = NumberToString(p->Score());
		money.caption.text = NumberToString(p->Money());
	}
}

void HUD::Draw()
{
	bool stat_changed = false;

	const BossInfo* boss_info = WorldBossGet();
	if (boss_info == nullptr)
	{
		if (boss_exists)
		{
			//There was a boss, but now it's gone - reset the health bar
			boss_exists = false;
			boss_bar.Reset();
		}
	}
	else
	{
		if (boss_name != boss_info->name)
		{
			//There is a boss in the level, but it is not the same as the old one - reset the health bar
			boss_bar.Reset();
			boss_name = boss_info->name;
		}

		boss_bar.Draw(boss_info->hp, boss_info->hp_max, boss_info->name, false);
		boss_exists = true;
	}

	PlayerStats *p = Player();
	if (p != nullptr)
	{
		health_bar.Draw(p->Shields(), p->ShieldsMaxValue(), EnvShieldMax());

		if (p->Score() != last_stats.Score())
		{
			stat_changed = true;
			score.caption.text = StringNumberFormat(p->Score());
		}

		//Draw text if multiplier is greater than 0
		score.hover_mouse = p->Multiplier() > 0;

		//For really high values of multiplier, highlight the text
		score.mousepressed = p->Multiplier() > limit;

		if (p->Multiplier() != last_stats.Multiplier())
		{
			stat_changed = true;
			score.tooltip.text = "x" + StringNumberFormat(p->Multiplier() + 1);
		}

		score.Draw();

		if (p->Money() != last_stats.Money())
		{
			stat_changed = true;
			money.caption.text = NumberToString(p->Money());
		}
		money.Draw();
	}

	if (p->Warranty())
		warranty.Draw();

	RenderSprite(weapon_primary.x, weapon_primary.y, weapon_primary.w, weapon_primary.h,
		p->Weapon(PLAYER_WEAPON_PRIMARY)->Hand(), p->Weapon(PLAYER_WEAPON_PRIMARY)->Info()->_color);

	RenderSprite(weapon_secondary.x, weapon_secondary.y, weapon_secondary.w, weapon_secondary.h,
		p->Weapon(PLAYER_WEAPON_SECONDARY)->Hand(), p->Weapon(PLAYER_WEAPON_SECONDARY)->Info()->_color);

	//Draw the compass
	if (p->Ability(ABILITY_COMPASS))
	{
		float angle = 0.0f;
		CompassState c = PlayerCompass(angle);
		if (c == COMPASS_ON)
			compass.Draw(angle);
		else if (c == COMPASS_EXIT)
			compass.DrawExit();
	}

	//The reason these are in an if/else condition is that both will usually be drawn in the
	//same location and we don't want any overlap. Logically, you first display the name of a
	//world and then when the player empties it, you display "zone clear!"

	if (WorldTitleVisible())
	{
		world_title.SetLevelTitle();
		world_title.Draw();
	}
	else if (WorldZoneClear())
		zone_clear.Draw();

	if (stat_changed)
		last_stats = *p;
}

void HUD::SetUI()
{
	health_bar.SetUI();
	boss_bar.SetUI();
	world_title.SetUI();
	zone_clear.SetUI();

	warranty.SetUI();
	weapon_primary.SetUI();
	weapon_secondary.SetUI();

	score.SetUI();
	money.SetUI();
	compass.SetUI();
}