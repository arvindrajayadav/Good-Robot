/*-----------------------------------------------------------------------------

  Page_pattern.cpp

  This module is a container for the massive level-generating code, which was
  too large and ungainly to exist with the rest of the Page class.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "env.h"
#include "fx.h"
#include "noise.h"
#include "page.h"
#include "random.h"
#include "TMXMap.h"

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void Page::BuildPattern()
{
	using namespace pyrodactyl;

	int               x, y;
	GLcoord2          world;
	int               column, row;
	vector<DoorInfo>  door_positions;
	TMXMap tmx;

	world = _grid * PAGE_SIZE;
	column = _grid.x;
	row = _grid.y;
	for (x = 0; x < PAGE_SIZE; x++) {
		for (y = 0; y < PAGE_SIZE; y++) {
			_variant[x][y] = (unsigned char)RandomVal(Env().tile_variants);
		}
	}
	//Fill in with solid mass. We'll dig tunnels in this below.
	for (x = 0; x < PAGE_SIZE; x++) {
		for (y = 0; y < PAGE_SIZE; y++) {
			_map[x][y] = MAP_SOLID;
		}
	}
	if (_pattern == "invalid")
		return;
	//A small hole in the center of the page, leading to the doorways.
	//This is the only hard coded room right now
	if (_pattern == "doors")	{
		float       dist;
		GLvector2   from_center;
		float       quarter;
		int         outer_wall, inner_wall;

		quarter = PAGE_HALF;
		inner_wall = PAGE_HALF - (column % 3) - 2;
		outer_wall = PAGE_HALF + 2;
		for (x = 0; x < PAGE_SIZE; x++) {
			for (y = 0; y < PAGE_SIZE; y++) {
				from_center = GLvector2((float)x - PAGE_HALF, (float)y - PAGE_HALF);
				dist = from_center.Length();
				if (dist < PAGE_HALF / 2)
					Dig(x, y);
			}
		}
		//Open tunnels to neighbors.
		for (y = 0; y < PAGE_SIZE; y++) {
			for (x = PAGE_HALF - 2; x < PAGE_HALF + 2; x++) {
				if (y <= PAGE_HALF && (_connect & CONNECT_LEFT))
					Dig (y, x);
				if (y >= PAGE_HALF && (_connect & CONNECT_RIGHT))
					Dig (y, x);
				if (y <= PAGE_HALF && (_connect & CONNECT_UP))
					Dig (x, y);
				if (y >= PAGE_HALF && (_connect & CONNECT_DOWN))
					Dig (x, y);
			}
		}
		int platform_width = PAGE_HALF / 3;
		int platform_bottom = PAGE_HALF + 2 + RandomVal (2);

		for (y = PAGE_HALF; y <= platform_bottom; y++) {
			for (x = -platform_width; x <= platform_width; x++) {
				//If we're on the underside, then leave pieces off randomly to make it uneven.
				if (abs (x) == platform_width || y == platform_bottom) {
					if (RandomVal (3) != 0)
						continue;
				}
				Fill (PAGE_HALF + x, y);
			}
			platform_width -= 1 + RandomVal (2);
		}
	} else {//Not a door, load a TMX file.
		boost::filesystem::path file_path(LEVELS_DIR + _pattern + LEVELS_EXT);
		if (exists(file_path)) {
			//Load the TMX file specified
			tmx.Load(LEVELS_DIR, _pattern + LEVELS_EXT);
		} else {
			//Load the default file
			tmx.Load(LEVELS_DIR, LEVELS_DEFAULT);
		}
		tmx.Copy(_map);
		//Bore tunnels to connect this level with the one above or below it.
		if (_connect) {
			if (_connect & CONNECT_LEFT)
				tmx.CreateDoor (DIRECTION_LEFT, _map);
			if (_connect & CONNECT_RIGHT)
				tmx.CreateDoor (DIRECTION_RIGHT, _map);
			if (_connect & CONNECT_UP)
				tmx.CreateDoor (DIRECTION_UP, _map);
			if (_connect & CONNECT_DOWN)
				tmx.CreateDoor (DIRECTION_DOWN, _map);
		}
	}

	DoAccess();
	DoSpawns();
	DoDoors(_desired_doors, tmx);
	//Fill in the shapes on the inside of this page. We can't do the edge ones yet,
	//because our neighbor pages might not be initialized yet.
	for (int x = 1; x < PAGE_EDGE; x++) {
		for (int y = 1; y < PAGE_EDGE; y++) {
			short shape = 0;
			if (_map[x][y])
				shape |= 1;
			if (_map[x + 1][y])
				shape |= 2;
			if (_map[x + 1][y + 1])
				shape |= 4;
			if (_map[x][y + 1])
				shape |= 8;
			_shape[x][y] = shape;
		}
	}
	DoLocations();
}

enum Doorways
{
	DOOR_UP1,
	DOOR_UP2,
	DOOR_UP3,
	DOOR_RIGHT1,
	DOOR_RIGHT2,
	DOOR_RIGHT3,
	DOOR_DOWN1,
	DOOR_DOWN2,
	DOOR_DOWN3,
	DOOR_LEFT1,
	DOOR_LEFT2,
	DOOR_LEFT3,
};

void Page::DoDoors(int doors, pyrodactyl::TMXMap &tmx)
{
	vector<Doorways>  ways;
	vector<Doorways>  chosen;

	if (!doors)
		return;
	//First compile our list of AVAILABLE doorways.
	if (_connect & DOORS_UP) {
		ways.push_back(DOOR_UP1);
		ways.push_back(DOOR_UP2);
		ways.push_back(DOOR_UP3);
	}
	if (_connect & DOORS_RIGHT) {
		ways.push_back(DOOR_RIGHT1);
		ways.push_back(DOOR_RIGHT2);
		ways.push_back(DOOR_RIGHT3);
	}
	if (_connect & DOORS_DOWN) {
		ways.push_back(DOOR_DOWN1);
		ways.push_back(DOOR_DOWN2);
		ways.push_back(DOOR_DOWN3);
	}
	if (_connect & DOORS_LEFT) {
		ways.push_back(DOOR_LEFT1);
		ways.push_back(DOOR_LEFT2);
		ways.push_back(DOOR_LEFT3);
	}
	//This can only happen during testing, not during normal gameplay.
	if (ways.empty())
		return;
	//Now choose a few from the ones available.
	for (int i = 0; i < doors; i++) {
		int rando = RandomVal(ways.size());
		chosen.push_back(ways[rando]);
		ways.erase(ways.begin() + rando);
	}
	//Now go over the list of selected door positions and dig tunnels through the geometry to make room for them.
	for (unsigned i = 0; i < chosen.size(); i++) {
		DoorInfo    di;
		int         offset;

		switch (chosen[i]) {
		case DOOR_RIGHT1:
		case DOOR_RIGHT2:
		case DOOR_RIGHT3:
			offset = PAGE_HALF;
			if (chosen[i] == DOOR_RIGHT2)
				offset -= 3;
			else if (chosen[i] == DOOR_RIGHT3)
				offset += 3;
			di.facing = DOOR_LEFT;
			di.position = _origin + GLvector2((float)PAGE_SIZE - PAGE_QUARTER, (float)offset);
			_door_info.push_back(di);
			_spawn_slots.push_back(GLcoord2(PAGE_SIZE - PAGE_QUARTER - 1, offset));
			for (int x = 0; x < PAGE_QUARTER; x++)
				Dig(PAGE_EDGE - x, offset);
			break;
		case DOOR_LEFT1:
		case DOOR_LEFT2:
		case DOOR_LEFT3:
			offset = PAGE_HALF;
			if (chosen[i] == DOOR_LEFT2)
				offset -= 3;
			else if (chosen[i] == DOOR_LEFT3)
				offset += 3;
			di.facing = DOOR_RIGHT;
			di.position = _origin + GLvector2((float)PAGE_QUARTER, (float)offset);
			_door_info.push_back(di);
			_spawn_slots.push_back(GLcoord2(PAGE_QUARTER + 1, offset));
			for (int x = 0; x <= PAGE_QUARTER; x++)
				Dig(x, offset);
			break;
		case DOOR_DOWN1:
		case DOOR_DOWN2:
		case DOOR_DOWN3:
			offset = PAGE_HALF;
			if (chosen[i] == DOOR_DOWN2)
				offset -= 3;
			else if (chosen[i] == DOOR_DOWN3)
				offset += 3;
			di.facing = DOOR_UP;
			di.position = _origin + GLvector2((float)offset, (float)PAGE_SIZE - PAGE_QUARTER);
			_door_info.push_back(di);
			_spawn_slots.push_back(GLcoord2(offset, PAGE_SIZE - PAGE_QUARTER - 1));
			for (int x = 0; x < PAGE_QUARTER; x++)
				Dig(offset, PAGE_EDGE - x);
			break;
		case DOOR_UP1:
		case DOOR_UP2:
		case DOOR_UP3:
			offset = PAGE_HALF;
			if (chosen[i] == DOOR_UP2)
				offset -= 3;
			else if (chosen[i] == DOOR_UP3)
				offset += 3;
			di.facing = DOOR_DOWN;
			di.position = _origin + GLvector2((float)offset, (float)PAGE_QUARTER);
			_door_info.push_back(di);
			_spawn_slots.push_back(GLcoord2(offset, PAGE_QUARTER + 1));
			for (int x = 0; x < PAGE_HALF; x++)
				Dig(offset, x);
			break;
		}
	}
}