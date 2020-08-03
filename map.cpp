/*-----------------------------------------------------------------------------

  Map.cpp

  Reads in level data files and converts them into structs that are used
  by world.cpp. No geometry is created here. This is purely conceptual.
  Here we make a list of screen positions and layouts, along with the types
  of bots that will inhabit them.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "bodyparts.h"
#include "env.h"
#include "fx.h"
#include "game.h"
#include "ini.h"
#include "loaders.h"
#include "map.h"
#include "random.h"
#include "resource.h"
#include "page.h"
#include "world.h"
#include "player.h"

using namespace pyrodactyl;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static ForcefieldType get_forcefield(string type)
{
	type = StringToLower(type);
	if (!type.compare("boss"))
		return FORCEFIELD_BOSS;
	if (!type.compare("mobs"))
		return FORCEFIELD_MOBS;
	return FORCEFIELD_NONE;
}

static vector<std::string> get_patterns(string data)
{
	vector<std::string>  result;
	vector<string>        list;

	list = StringSplit(data, " ,");
	for (unsigned i = 0; i < list.size(); i++)
	{
		boost::filesystem::path file_path(LEVELS_DIR + list[i] + LEVELS_EXT);
		if (exists(file_path))
			result.push_back(list[i]);
		else
			Console("Unknown screen pattern '%s'.", list[i].c_str());
	}

	if (result.empty())
		result.push_back(LEVELS_DEFAULT);

	return result;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static string string_from_xml(rapidxml::xml_node<char> *node, string field)
{
	string result;

	LoadStr(result, field, node);
	return result;
}

bool mob_from_xml(rapidxml::xml_node<char> *node, int index, ZoneMob& mob)
{
	rapidxml::xml_node<char> *mob_node = node->first_node(StringSprintf("Mob%d", index).c_str());
	if (!NodeValid(mob_node))
		return false;
	string name = string_from_xml(mob_node, "Robot");
	mob.type_index = EnvRobotIndexFromName(name);
	if (mob.type_index == 0) {
		Console("WARNING: Unknown Robot type '%s' in levels.xml", name.c_str());
		return false;
	}
	mob.count_min = StringToInt(string_from_xml(mob_node, "Min"));
	mob.count_max = StringToInt(string_from_xml(mob_node, "Max"));
	mob.from_machine = StringToLower(string_from_xml(mob_node, "Source")).compare("machine") == 0;
	if (!LoadNum(mob.room_index, "Room", mob_node))
		mob.room_index = -1;
	return true;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void Motif::Init(void *mnode_ptr)
{
	rapidxml::xml_node<char>  *mnode = (rapidxml::xml_node<char>*)mnode_ptr;

	//Read in the texture names.
	if (NodeValid("Textures", mnode))	{
		rapidxml::xml_node<char> *texture_node = mnode->first_node("Textures");
		_texture_fore = string_from_xml(texture_node, "Foreground");
		_texture_mid = string_from_xml(texture_node, "Middle");
		_texture_back = string_from_xml(texture_node, "Background");
		_texture_sky = string_from_xml(texture_node, "Sky");
	}
	_color[COLOR_FOREGROUND] = GLrgbaFromHex(string_from_xml(mnode, "Foreground"));
	_color[COLOR_BACKGROUND1] = GLrgbaFromHex(string_from_xml(mnode, "Middle"));
	_color[COLOR_BACKGROUND2] = GLrgbaFromHex(string_from_xml(mnode, "Background"));
	_color[COLOR_SKY] = GLrgbaFromHex(string_from_xml(mnode, "Sky"));
	_color[COLOR_LAMP] = GLrgbaFromHex(string_from_xml(mnode, "Lamp"));
	_color[COLOR_LIGHT] = GLrgbaFromHex(string_from_xml(mnode, "Light"));
	_fog = 1.0f - StringToFloat(string_from_xml(mnode, "Fog"));
	_wall_damage = StringToInt(string_from_xml(mnode, "WallDamage"));
	_machines = StringSplit(string_from_xml(mnode, "Machines"), " ,");
	_blind = StringToInt(string_from_xml(mnode, "Blind")) != 0;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

string Map::TextureName(MapTexture t)
{
	if (t == TEXTURE_FRONT)
		return _texture_fore;
	if (t == TEXTURE_MIDDLE)
		return _texture_mid;
	if (t == TEXTURE_BACK)
		return _texture_back;
	return _texture_sky;
}

const struct Motif* Map::RandomMotif()
{
	int n = RandomVal(_motifs.size());
	return &_motifs[n];
}

const struct Motif* Map::GetMotif(int index)
{
	return &_motifs[index];
}

const char* Map::Music(int zone_num)
{
	if (!_zones[zone_num]._music.empty())
		return _zones[zone_num]._music.c_str();
	return _music.c_str();
}

void Map::Init(int index)
{
	XMLDoc level_file(ResourceLocation(LEVELS_XML, RESOURCE_DATA));

	_zones.clear();
	if (!level_file.ready())
		return;
	rapidxml::xml_node<char> *node = level_file.Doc()->first_node("levels");
	if (!NodeValid(node))
		return;

	rapidxml::xml_node<char> *level_node = node->first_node(StringSprintf("Level%d", index).c_str());
	if (!NodeValid(level_node))
		return;

	//Read the overall properties of this particular level.
	if (NodeValid("Properties", level_node)) {
		rapidxml::xml_node<char> *prop_node = level_node->first_node("Properties");
		_title = string_from_xml(prop_node, "Title");
		_subtitle = string_from_xml(prop_node, "Subtitle");
		_music = string_from_xml(prop_node, "Music");
		_min_zones = StringToInt(string_from_xml(prop_node, "ZonesMin"));
		_max_zones = StringToInt(string_from_xml(prop_node, "ZonesMax"));
		_fog = 1.0f - StringToFloat(string_from_xml(prop_node, "Fog"));
		_fog = clamp(_fog, 0.0f, 1.0f);
	}

	//Read in the texture names.
	if (NodeValid("Textures", level_node))	{
		rapidxml::xml_node<char> *texture_node = level_node->first_node("Textures");
		_texture_fore = string_from_xml(texture_node, "Foreground");
		_texture_mid = string_from_xml(texture_node, "Middle");
		_texture_back = string_from_xml(texture_node, "Background");
		_texture_sky = string_from_xml(texture_node, "Sky");
	}

	///Get the motifs
	_motifs.clear();
	rapidxml::xml_node<char> *mnode;
	for (unsigned i = 0;; i++) {
		Motif  motif;

		mnode = level_node->first_node(StringSprintf("Motif%d", i).c_str());
		if (!NodeValid(mnode))
			break;
		motif.Init(mnode);
		_motifs.push_back(motif);
	}
	//If no motif is given, create a placeholder.
	if (_motifs.empty()) {
		Motif  motif;
		for (int i = 0; i < COLOR_COUNT; i++) {
			motif._color[i] = GLrgba(0.5f, 0.5f, 0.5f);
		}
		motif._fog = 0.5f;
		_motifs.push_back(motif);
	}
	//Read in the zones.
	rapidxml::xml_node<char> *znode;
	for (unsigned i = 0;; i++) {
		ZoneInfo    zi;
		PlayerZoneInfo p_zi;

		znode = level_node->first_node(StringSprintf("Zone%d", i).c_str());
		if (!NodeValid(znode))
			break;
		zi._map_id = index;
		zi._zone_id = i;
		zi._door_sprite = SpriteEntryLookup(string_from_xml(znode, "Door"));
		zi._length = StringToInt(string_from_xml(znode, "Rooms"));
		zi._patterns = get_patterns(string_from_xml(znode, "Patterns"));
		zi._music = string_from_xml(znode, "Music");
		zi._end_machine = string_from_xml(znode, "EndMachine");
		zi._has_motif = false;
		zi._forcefield = get_forcefield(string_from_xml(znode, "Forcefield"));

		//See if this zone has a fixed motif.
		rapidxml::xml_node<char> *mnode;
		mnode = znode->first_node("Motif");
		if (NodeValid(mnode)) {
			zi._has_motif = true;
			zi._motif.Init(mnode);
		}
		p_zi._map_id = index;
		p_zi._zone_id = i;
		p_zi._is_complete = false;

		//Get the list of robots for this zone.
		for (unsigned m = 0;; m++)
		{
			ZoneMob zm;
			if (!mob_from_xml(znode, m, zm))
				break;
			zi._mobs.push_back(zm);
		}
		zi._is_combat = !zi._mobs.empty();

		Player()->ZoneAdd(p_zi, false);
		_zones.push_back(zi);
	}
	//If min / max zones aren't set in the properties...
	if (_min_zones == 0 && _max_zones == 0) {
		_min_zones = _zones.size() / 2; //make player do at least half the zones.
		_max_zones = _zones.size() - 1;
	}
}