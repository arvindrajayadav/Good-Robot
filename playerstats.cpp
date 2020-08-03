/*-----------------------------------------------------------------------------

  Playerstats.cpp

  This handles the mechanical statistics of the player.  This class is what
  ends up in our save game file: Shields, energy, skills, checkpoints, etc.

  Player.cpp handles the input and movement. Avatar.cpp moves and animates the
  player character. And Playerstats.cpp has the save game info.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "character.h"
#include "entity.h"
#include "env.h"
#include "file.h"
#include "fx.h"
#include "ini.h"
#include "lootpool.h"
#include "player.h"
#include "random.h"
#include "system.h"
#include "world.h"
#include "loaders.h"
#include "stringpool.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#define POINTS_INC_MAX 8000

  /*-----------------------------------------------------------------------------

  -----------------------------------------------------------------------------*/

bool PlayerStats::Ability(int index)
{
	if (index > -1 && index < ABILITY_TYPES)
		return _ability[index];

	return false;
}

void PlayerStats::Ability(int index, bool val)
{
	if (index > -1 && index < ABILITY_TYPES)
		_ability[index] = val;
}

void PlayerStats::Init(int character, eGameMode gm)
{
	//We use the same difficulty as before
	bool difficulty = _difficulty;

	memset(this, 0, sizeof(PlayerStats));
	_difficulty = difficulty;
	_version = SAVE_FILE_VERSION;
	_game_mode = gm;
	_multiplier = 0;
	_multiplier_timeout = 0;
	for (int i = 0; i < ABILITY_TYPES; i++)
		_ability[i] = false;
	_character = character;
	_score_points = 0l;
	_timestamp_start = SystemTime();
	_shield_current = ShieldsMaxValue();
	_color = GLrgba(0.5f, 0.5f, 0.5f);
	_warranty = false;
	_warranty_count = 0;
	_upgrade_count = 0;
	_hat.level = -1;
	_hat.zone = -1;
	//Select a couple of random weapons.
	vector<int>   list_master;
	vector<int>   list_primary;
	vector<int>   list_secondary;

	list_master = LootpoolFromName("weaponsstart");
	for (unsigned i = 0; i < list_master.size(); i++) {
		const Projectile*   p = EnvProjectileFromId(list_master[i]);

		if (p->_slot == PROJECTILE_SECONDARY)
			list_secondary.push_back(list_master[i]);
		else
			list_primary.push_back(list_master[i]);
	}
	//Now we have two lists, one for each weapon type. Randomly select one
	//from each list.
	const Projectile*   primary;
	const Projectile*   secondary;
	primary = EnvProjectileFromId(list_primary[RandomVal(list_primary.size())]);
	secondary = EnvProjectileFromId(list_secondary[RandomVal(list_secondary.size())]);
	_weapon[PLAYER_WEAPON_PRIMARY].Equip(primary);
	_weapon[PLAYER_WEAPON_SECONDARY].Equip(secondary);
}

int PlayerStats::ShieldsMaxValue()
{
	return EnvShieldStrength(_skill[SKILL_SHIELDS]).val;
}

float PlayerStats::VisionRadius()
{
	return EnvVisionRadius(_skill[SKILL_VISION_RADIUS]).val;
}

void PlayerStats::EventRobotsKill(int count)
{
	_multiplier = min(_multiplier + count, Env().multiplier_max);
	_multiplier_timeout = GameTick() + Env().multiplier_timeout[_multiplier];
	TriviaModify(TRIVIA_KILLS, count);
}

void PlayerStats::EventMissileKill()
{
	TriviaModify(TRIVIA_MISSILES_DESTROYED, 1);
}

void PlayerStats::SkillSet(PlayerSkill index, int val)
{
	_skill[index] = clamp(val, 0, (int)EnvSkillMaxLevel(index));
}

void PlayerStats::SkillUp(PlayerSkill cat)
{
	if (!SkillUpEligible(cat))
		return;

	_upgrade_count++;
	_skill[cat]++;

	SkillCheckAch(cat);

	if (cat == SKILL_SHIELDS)
		RestoreShields();
}

void PlayerStats::ShieldsSet(int val)
{
	_shield_current = clamp(val, 0, ShieldsMaxValue());
}

void PlayerStats::ShieldsModify(int change)
{
	//If we take damage, it resets our multiplier.
	if (change < 0) {
		_multiplier = 0;
	}
	_shield_current += change;
	_shield_current = clamp(_shield_current, 0, ShieldsMaxValue());
}

float PlayerStats::ShieldsPercent()
{
	return (float)_shield_current / (float)ShieldsMaxValue();
}

bool PlayerStats::ShieldsFull()
{
	return _shield_current == ShieldsMaxValue();
}

void PlayerStats::RestoreShields()
{
	_shield_current = ShieldsMaxValue();
}

void PlayerStats::MoneyGive(int val)
{
	_money += val;
	if (val > 0)
		TriviaModify(TRIVIA_XP_GATHERED, val);
}

void PlayerStats::CheckpointSet(Checkpoint location)
{
	_checkpoint = location;
	_highest_map = max(_highest_map, location.map);
}

void PlayerStats::Kill(GLvector2 position, GLvector2 momentum)
{
	bool                eliminate;
	vector<ePickupType> hold;
	vector<ePickupType> drop;

	_shield_current = 0;
	eliminate = true;
	TriviaModify(TRIVIA_DEATHS, 1);
}

void PlayerStats::Update()
{
	if (GameTick() > _multiplier_timeout) {
		_multiplier = 0;
	}
}

void PlayerStats::ScorePoints(int pts)
{
	long result = pts * (_multiplier + 1);

	//Limit the maximum number of points gained to avoid abnormally high score values
	if (result > POINTS_INC_MAX)
		result = POINTS_INC_MAX;

	_score_points += result;
	ScoreCheckAch();
}

void PlayerStats::ZoneAdd(const PlayerZoneInfo &pzi, const bool overwrite)
{
	//First check if a zone with the same map and zone id already exists
	if (!_zone_info.empty())
	{
		for (unsigned int i = 0; i < _zone_info.size(); ++i)
			if (_zone_info[i]._map_id == pzi._map_id && _zone_info[i]._zone_id == pzi._zone_id)
			{
				//overwrite = true for when we want our new data to take preference over the old data, ex. loading from save data
				if (overwrite)
				{
					//Just update the data inside the zone itself
					_zone_info[i]._is_complete = pzi._is_complete;
				}
				else
				{
					//This is for when we want existing data to take preference over the old data, ex. initializing a level
					_zone_info[i]._is_complete = _zone_info[i]._is_complete || pzi._is_complete;
				}
				return;
			}
	}
	//This zone does not exist, add it to our list
	_zone_info.push_back(pzi);
}

bool PlayerStats::ZoneComplete(const int map_id, const int zone_id)
{
	//First check if a zone with the same map and zone id already exists
	if (!_zone_info.empty())
	{
		for (unsigned int i = 0; i < _zone_info.size(); ++i)
			if (_zone_info[i]._map_id == map_id && _zone_info[i]._zone_id == zone_id)
				return _zone_info[i]._is_complete;
	}

	//A zone like this does not exist, so we assume we haven't completed it
	return false;
}

bool PlayerStats::Load(string filename)
{
	//Reset the multiplier, this is not loaded from file
	_multiplier = 0;
	_multiplier_timeout = 0;

	using namespace pyrodactyl;
	XMLDoc doc(filename);
	if (doc.ready())
	{
		rapidxml::xml_node<char> *node = doc.Doc()->first_node("player");
		if (NodeValid(node))
		{
			LoadNum(_version, "version", node);
			if (!_version)
				return false;

			if (NodeValid("time", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("time");
				LoadNum(_timestamp_start, "start", n);
				LoadNum(_timestamp_last, "last", n);
			}

			if (NodeValid("game", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("game");
				LoadEnum(_game_mode, "mode", n);
				LoadNum(_score_points, "score", n);
				LoadBool(_difficulty, "diff", n);
			}

			if (NodeValid("character", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("character");
				LoadEnum(_character, "val", n);

				std::string col;
				LoadStr(col, "color", n);
				_color = GLrgbaFromHex(col);
			}

			if (NodeValid("checkpoint", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("checkpoint");
				LoadEnum(_checkpoint.map, "map", n);
				LoadNum(_checkpoint.zone, "zone", n);
				LoadNum(_checkpoint.page, "page", n);
				LoadNum(_highest_map, "highest_map", n);

				if (NodeValid("zones", n))
				{
					rapidxml::xml_node<char> *zonode = n->first_node("zones");
					for (rapidxml::xml_node<char> *i = zonode->first_node(); i != nullptr; i = i->next_sibling())
					{
						PlayerZoneInfo pzi;
						bool result = LoadNum(pzi._map_id, "map", i);
						result = result && LoadNum(pzi._zone_id, "zone", i);
						result = LoadBool(pzi._is_complete, "complete", i);
						_zone_info.push_back(pzi);
					}
				}
			}

			if (NodeValid("shields", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("shields");
				LoadNum(_shield_current, "val", n);
			}

			if (NodeValid("xp", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("xp");
				LoadNum(_money, "val", n);
			}

			if (NodeValid("skill", node))
			{
				rapidxml::xml_node<char> *sknode = node->first_node("skill");
				int i = 0;

				for (rapidxml::xml_node<char> *n = sknode->first_node(); n != nullptr && i < SKILL_COUNT; n = n->next_sibling(), ++i)
					LoadNum(_skill[i], "val", n);
			}

			if (NodeValid("ability", node))
			{
				rapidxml::xml_node<char> *abnode = node->first_node("ability");
				int i = 0;

				for (rapidxml::xml_node<char> *n = abnode->first_node(); n != nullptr && i < ABILITY_TYPES; n = n->next_sibling(), ++i)
					LoadBool(_ability[i], "val", n);
			}

			if (NodeValid("trivia", node))
			{
				rapidxml::xml_node<char> *trnode = node->first_node("trivia");
				int i = 0;

				for (rapidxml::xml_node<char> *n = trnode->first_node(); n != nullptr && i < TRIVIA_COUNT; n = n->next_sibling(), ++i)
					LoadNum(_trivia[i], "val", n);
			}

			if (NodeValid("weapon", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("weapon");
				std::string p, s;

				LoadNum(p, "primary", n);
				LoadNum(s, "secondary", n);
				LoadNum(_weapon_purchase_count, "count", n);

				_weapon[PLAYER_WEAPON_PRIMARY].Equip(EnvProjectileFromName(p));
				_weapon[PLAYER_WEAPON_SECONDARY].Equip(EnvProjectileFromName(s));
			}

			if (NodeValid("warranty", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("warranty");
				LoadBool(_warranty, "val", n);
				LoadNum(_warranty_count, "count", n);
			}

			if (NodeValid("repair", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("repair");
				LoadNum(_repair_count, "count", n);
			}

			if (NodeValid("upgrade", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("upgrade");
				LoadNum(_upgrade_count, "count", n);
			}

			if (NodeValid("hat", node))
			{
				rapidxml::xml_node<char> *n = node->first_node("hat");
				LoadNum(_hat.level, "level", n);
				LoadNum(_hat.zone, "zone", n);
				LoadNum(_hat.purchase_count, "count", n);
				LoadNum(_hat.size, "size", n);
				LoadStr(_hat.sprite, "sprite", n);
				LoadStr(_hat.name, "name", n);
				LoadBool(_hat.wearing, "wearing", n);
				LoadBool(_hat.purchased_this_zone, "purchased_this_zone", n);
				LoadColor(_hat.color, n);

				if (_hat.wearing)
					PlayerHatInit();
			}
		}
	}

	return true;
}

void PlayerStats::Save(string filename)
{
	using namespace pyrodactyl;

	rapidxml::xml_document<char> doc;

	//XML declaration
	{
		rapidxml::xml_node<char> *decl = doc.allocate_node(rapidxml::node_declaration);
		decl->append_attribute(doc.allocate_attribute("version", "1.0"));
		decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
		doc.append_node(decl);
	}

	//Start of writing data to file

	//Root node
	rapidxml::xml_node<char> *root = doc.allocate_node(rapidxml::node_element, "player");
	root->append_attribute(doc.allocate_attribute("version", gStrPool.Get(SAVE_FILE_VERSION)));
	doc.append_node(root);

	//Time
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "time");
		child->append_attribute(doc.allocate_attribute("start", gStrPool.Get(_timestamp_start)));
		child->append_attribute(doc.allocate_attribute("last", gStrPool.Get(SystemTime())));
		root->append_node(child);
	}

	//Game
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "game");
		child->append_attribute(doc.allocate_attribute("mode", gStrPool.Get(_game_mode)));
		child->append_attribute(doc.allocate_attribute("score", gStrPool.LGet(_score_points)));
		SaveBool(_difficulty, "diff", doc, child);
		root->append_node(child);
	}

	//Character
	std::string col = GLrgbaToHex(_color);
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "character");
		child->append_attribute(doc.allocate_attribute("val", gStrPool.Get(_character)));
		child->append_attribute(doc.allocate_attribute("color", col.c_str()));
		root->append_node(child);
	}

	//Checkpoint
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "checkpoint");
		child->append_attribute(doc.allocate_attribute("map", gStrPool.Get(_checkpoint.map)));
		child->append_attribute(doc.allocate_attribute("zone", gStrPool.Get(_checkpoint.zone)));
		child->append_attribute(doc.allocate_attribute("page", gStrPool.Get(_checkpoint.page)));
		child->append_attribute(doc.allocate_attribute("highest_map", gStrPool.Get(_highest_map)));

		rapidxml::xml_node<char> *gr_child = doc.allocate_node(rapidxml::node_element, "zones");
		for (unsigned int i = 0; i < _zone_info.size(); ++i)
		{
			rapidxml::xml_node<char> *gr2_child = doc.allocate_node(rapidxml::node_element, "z");
			gr2_child->append_attribute(doc.allocate_attribute("map", gStrPool.Get(_zone_info[i]._map_id)));
			gr2_child->append_attribute(doc.allocate_attribute("zone", gStrPool.Get(_zone_info[i]._zone_id)));
			SaveBool(_zone_info[i]._is_complete, "complete", doc, gr2_child);
			gr_child->append_node(gr2_child);
		}
		child->append_node(gr_child);

		root->append_node(child);
	}

	//Shields
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "shields");
		child->append_attribute(doc.allocate_attribute("val", gStrPool.Get(_shield_current)));
		root->append_node(child);
	}

	//XP
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "xp");
		child->append_attribute(doc.allocate_attribute("val", gStrPool.LGet(_money)));
		root->append_node(child);
	}

	//Skill
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "skill");
		for (unsigned i = 0; i < SKILL_COUNT; i++)
		{
			rapidxml::xml_node<char> *gr_child = doc.allocate_node(rapidxml::node_element, "s");
			gr_child->append_attribute(doc.allocate_attribute("id", gStrPool.Get(i)));
			gr_child->append_attribute(doc.allocate_attribute("val", gStrPool.Get(_skill[i])));
			child->append_node(gr_child);
		}

		root->append_node(child);
	}

	//Ability
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "ability");
		for (unsigned i = 0; i < ABILITY_TYPES; i++)
		{
			rapidxml::xml_node<char> *gr_child = doc.allocate_node(rapidxml::node_element, "a");
			gr_child->append_attribute(doc.allocate_attribute("id", gStrPool.Get(i)));
			SaveBool(_ability[i], "val", doc, gr_child);
			child->append_node(gr_child);
		}

		root->append_node(child);
	}

	//Trivia
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "trivia");
		for (unsigned i = 0; i < TRIVIA_COUNT; i++)
		{
			rapidxml::xml_node<char> *gr_child = doc.allocate_node(rapidxml::node_element, "t");
			gr_child->append_attribute(doc.allocate_attribute("id", gStrPool.Get(i)));
			gr_child->append_attribute(doc.allocate_attribute("val", gStrPool.Get(_trivia[i])));
			child->append_node(gr_child);
		}

		root->append_node(child);
	}

	//Weapon
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "weapon");
		child->append_attribute(doc.allocate_attribute("primary", _weapon[PLAYER_WEAPON_PRIMARY].Info()->_name.c_str()));
		child->append_attribute(doc.allocate_attribute("secondary", _weapon[PLAYER_WEAPON_SECONDARY].Info()->_name.c_str()));
		child->append_attribute(doc.allocate_attribute("count", gStrPool.Get(_weapon_purchase_count)));
		root->append_node(child);
	}

	//Warranty
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "warranty");
		SaveBool(_warranty, "val", doc, child);
		child->append_attribute(doc.allocate_attribute("count", gStrPool.Get(_warranty_count)));
		root->append_node(child);
	}

	//Repair
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "repair");
		child->append_attribute(doc.allocate_attribute("count", gStrPool.Get(_repair_count)));
		root->append_node(child);
	}

	//Upgrade
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "upgrade");
		child->append_attribute(doc.allocate_attribute("count", gStrPool.Get(_upgrade_count)));
		root->append_node(child);
	}

	//Hat
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "hat");
		child->append_attribute(doc.allocate_attribute("level", gStrPool.Get(_hat.level)));
		child->append_attribute(doc.allocate_attribute("zone", gStrPool.Get(_hat.zone)));
		child->append_attribute(doc.allocate_attribute("count", gStrPool.Get(_hat.purchase_count)));
		child->append_attribute(doc.allocate_attribute("sprite", _hat.sprite.c_str()));
		child->append_attribute(doc.allocate_attribute("name", _hat.name.c_str()));
		child->append_attribute(doc.allocate_attribute("size", gStrPool.FGet(_hat.size)));

		child->append_attribute(doc.allocate_attribute("r", gStrPool.Get((int)(_hat.color.red * 255))));
		child->append_attribute(doc.allocate_attribute("g", gStrPool.Get((int)(_hat.color.green * 255))));
		child->append_attribute(doc.allocate_attribute("b", gStrPool.Get((int)(_hat.color.blue * 255))));
		child->append_attribute(doc.allocate_attribute("a", gStrPool.Get((int)(_hat.color.alpha * 255))));

		SaveBool(_hat.wearing, "wearing", doc, child);
		SaveBool(_hat.purchased_this_zone, "purchased_this_zone", doc, child);
		root->append_node(child);
	}

	//End of writing data to file

	//Put all this to a string
	std::string xml_as_string;
	rapidxml::print(std::back_inserter(xml_as_string), doc);

	//Save to file, then delete the XML data
	FileSave(filename, xml_as_string.c_str(), xml_as_string.length());
	doc.clear();
}