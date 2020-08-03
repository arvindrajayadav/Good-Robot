#include "master.h"
#include "TMXMap.h"
#include "loaders.h"
#include "page.h"
#include "random.h"

using namespace pyrodactyl;

TMXMap::TMXMap()
{
	tile_rows = 0;
	tile_cols = 0;
}

//------------------------------------------------------------------------
// Purpose: Load stuff via a .tmx file set to XML storage (no compression)
//------------------------------------------------------------------------
void TMXMap::Load(const std::string &path, std::string filename)
{
	XMLDoc conf(path + filename);
	if (conf.ready())
	{
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("map");
		if (NodeValid(node))
		{
			LoadNum(tile_rows, "width", node);
			LoadNum(tile_cols, "height", node);

			if (NodeValid("layer", node))
			{
				rapidxml::xml_node<char> *groupnode = node->first_node("layer");
				layer.Load(path, groupnode);
			}
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Copy our TMX data to the game's map format
// The weird declaration exists to enforce array size
//------------------------------------------------------------------------
void TMXMap::Copy(unsigned char(&map)[PAGE_SIZE][PAGE_SIZE])
{
	//Roll the dice once for tile id 5 and 6
	//These tiles are synchronized on/off
	int dice[2] = { 0, 0 };
	dice[0] = RandomVal(2);
	dice[1] = RandomVal(2);

	//Roll the dice for tile 7
	//Tile 8 is the inverse of tile 7
	int dice2 = RandomVal(2);

	for (int x = 0; x < PAGE_SIZE; x++)
	{
		for (int y = 0; y < PAGE_SIZE; y++)
		{
			switch (layer.tile[x][y].gid)
			{
			case 1:
				map[x][y] = MAP_SOLID;
				break;
			case 2:
				//Exit tiles, keep them solid for now
				map[x][y] = MAP_SOLID;
				break;
			case 4:
				//Randomly decide to either make this solid or open
				if (RandomVal(2) == 0) map[x][y] = MAP_SOLID; else map[x][y] = MAP_OPEN;
				break;
			case 5:
			case 6:
				if (dice[layer.tile[x][y].gid - 5] == 0) map[x][y] = MAP_SOLID; else map[x][y] = MAP_OPEN;
				break;
			case 7:
				if (dice2 == 0) map[x][y] = MAP_SOLID; else map[x][y] = MAP_OPEN;
				break;
			case 8:
				if (dice2 != 0) map[x][y] = MAP_SOLID; else map[x][y] = MAP_OPEN;
				break;
			default:
				//Note: tile id 3 is open
				map[x][y] = MAP_OPEN;
				break;
			}
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Dig a door in the map in a single direction
//------------------------------------------------------------------------
void TMXMap::CreateDoor(const PageTraverse &dir, unsigned char(&map)[PAGE_SIZE][PAGE_SIZE])
{
	switch (dir)
	{
	case DIRECTION_RIGHT:
		for (int x = (3 * PAGE_SIZE) / 4; x < PAGE_SIZE; ++x)
			for (int y = 0; y < PAGE_SIZE; ++y)
				if (layer.tile[x][y].gid == 2)
					map[x][y] = MAP_OPEN;
		break;
	case DIRECTION_LEFT:
		for (int x = 0; x <= PAGE_SIZE / 4; ++x)
			for (int y = 0; y < PAGE_SIZE; ++y)
				if (layer.tile[x][y].gid == 2)
					map[x][y] = MAP_OPEN;
		break;
	case DIRECTION_DOWN:
		for (int x = 0; x < PAGE_SIZE; ++x)
			for (int y = (3 * PAGE_SIZE) / 4; y < PAGE_SIZE; ++y)
				if (layer.tile[x][y].gid == 2)
					map[x][y] = MAP_OPEN;
		break;
	case DIRECTION_UP:
		for (int x = 0; x < PAGE_SIZE; ++x)
			for (int y = 0; y <= PAGE_SIZE / 4; ++y)
				if (layer.tile[x][y].gid == 2)
					map[x][y] = MAP_OPEN;
		break;
	}
}