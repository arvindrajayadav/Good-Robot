/*-----------------------------------------------------------------------------

  Page.cpp

  This class handles a grid of points that make up our cave. Many of these
  are stuck together to make up the world.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "bodyparts.h"
#include "env.h"
#include "fx.h"
#include "game.h"
#include "fxmachine.h"
#include "noise.h"
#include "random.h"
#include "render.h"
#include "page.h"
#include "world.h"
#include "zone.h"

#define ONE_BLOCK           (1.0f / PAGE_SIZE)
#define HALF_PIXEL          (ONE_BLOCK / 64)
#define DEBUG_COLORING      0
#define DEBUG_MESH          (DEBUG_SHOW_ACCESS || DEBUG_SHOW_SPAWN || DEBUG_SHOW_ICON)
#define DEBUG_SHOW_ACCESS   1
#define DEBUG_SHOW_SPAWN    1
#define DEBUG_SHOW_ICON     1
#define TILE_VARIANTS       Env ().tile_variants
#define TILE_HEIGHT         (1.0f / 8.0f)
#define RANDOM_SCALE_COUNT  (sizeof (random_scale) / sizeof (float))
#define OUTER_MOD           0.97f
#define INNER_MOD           0.85f

enum
{
	TILE_FLOOR,
	TILE_FLOOR_SLOPE,
	TILE_WALL,
	TILE_CEIL_SLOPE,
	TILE_CEIL,
	TILE_SOLID,
	TILE_SPECIAL,
	TILE_DOODAD,
};

enum
{
	TILE_SPECIAL_LIGHT,
	TILE_SPECIAL_BLACK,
};

static float  random_scale[] = { 1.0f, 2.0f, 1.33f, 2.2f, 1.6f, 1.9f, 1.1f };
static int    random_scale_index;
static int    doodad_variant;

/*-----------------------------------------------------------------------------
All tiles are stored on one large sheet. There are four shapes:
1) Ceiling 2) sloped ceiling 3) Wall 4) Sloped Floor 5) Floor
These four shapes are mirrored and shifted as needed to form the 16
permutations of marching squares.

The top half of the texture is the base wall. The bottom half is the matching
glowmap for having walls glow or "cast" light.
-----------------------------------------------------------------------------*/

GLuvFrame Page::GetUV(int vary, int shape_in, bool glow)
{
	GLuvFrame result;
	float     group;
	float     shape;
	GLvector2 size;
	GLvector2 origin;
	GLvector2 half_pixel;

	half_pixel = GLvector2(HALF_PIXEL, HALF_PIXEL);
	size = GLvector2((1.0f / (TILE_VARIANTS)), TILE_HEIGHT * 0.5f);
	group = size.x * (float)vary;
	shape = size.y * (float)shape_in;

	origin = GLvector2(group, shape);
	if (glow)
		origin.y += 0.5f;
	result.Set(origin + half_pixel, origin + size - half_pixel);
	return result;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

//Examine the given area of the page. If blocked = true, then return true
//ONLY if the entire area is SOLID. If blocked = false, then return true ONLY
//if the given area is open and not blocked by other machines.
bool Page::AreaScan(GLcoord2 start, GLcoord2 size, int desired_shape)
{
	GLcoord2    local;
	GLcoord2    end = start + size;;

	for (local.x = start.x; local.x < end.x; local.x++) {
		for (local.y = start.y; local.y < end.y; local.y++) {
			//Stay in bounds.
			if (local.x < 1 || local.x >= PAGE_EDGE || local.y < 1 || local.y >= PAGE_EDGE)
				return false;
			//If we're looking for solid ground...
			if (_shape[local.x][local.y] != desired_shape)
				return false;
			//We're looking for open space, but there's a machine here...
			if (desired_shape == 0 && _blocked[local.x][local.y])
				return false;
		}
	}
	return true;
}

/*-----------------------------------------------------------------------------
1--2
|  |
8--4
-----------------------------------------------------------------------------*/

#define SHAPE_CEILING     3
#define SHAPE_FLOOR       12
#define SHAPE_WALL_LEFT   9
#define SHAPE_WALL_RIGHT  6

GLcoord2 Page::MachineLocation (MachineMount mount, GLcoord2 size)
{
	GLcoord2    local;
	GLcoord2    chosen;
	bool        found;

	chosen = GLcoord2();
	found = false;
	for (local.x = 0; local.x < PAGE_SIZE; local.x++) {
		for (local.y = 0; local.y < PAGE_SIZE; local.y++) {
			//Check and make sure this area is open.
			if (!AreaScan(local, size, 0))
				continue;
			//Make sure the ground under us is flat floor.
			if (mount == MACHINE_MOUNT_FLOOR) {
				if (!AreaScan(GLcoord2(local.x, local.y + size.y), GLcoord2(size.x, 1), SHAPE_FLOOR))
					continue;
			}
			else if (mount == MACHINE_MOUNT_CEILING) { //Make sure there's a ceiling above us.
				if (!AreaScan(GLcoord2(local.x, local.y - 1), GLcoord2(size.x, 1), SHAPE_CEILING))
					continue;
			}
			else  if (mount == MACHINE_MOUNT_LEFT) {//Make sure there's a wall to our left
				if (!AreaScan(GLcoord2(local.x - 1, local.y), GLcoord2(1, size.y), SHAPE_WALL_LEFT))
					continue;
			}
			else if (mount == MACHINE_MOUNT_RIGHT) { //Make sure there's a wall to our right
				if (!AreaScan(GLcoord2(local.x + size.x, local.y), GLcoord2(1, size.y), SHAPE_WALL_RIGHT))
					continue;
			}
			chosen = local;
			found = true;
		}
	}
	if (!found)
		return GLcoord2();
	//We've found a spot. Mark it as occupied so no other machines can go here.
	for (local.x = chosen.x; local.x < chosen.x + size.x; local.x++) {
		for (local.y = chosen.y; local.y < chosen.y + size.y; local.y++) {
			_blocked[local.x][local.y] = true;
		}
	}
	return (_grid * PAGE_SIZE) + chosen;
}

/*-----------------------------------------------------------------------------
Each Corner has a value as thus, giving us 15 patterns:
1--2
|  |
8--4
-----------------------------------------------------------------------------*/

short Page::Shape(int world_x, int world_y)
{
	GLcoord2      local;
	short         shape;

	local.x = world_x % PAGE_SIZE;
	local.y = world_y % PAGE_SIZE;
	if (_shape[local.x][local.y] != SHAPE_INVALID)
		return _shape[local.x][local.y];
	shape = 0;
	if (WorldCellSolid(GLcoord2(world_x, world_y)))
		shape |= 1;
	if (WorldCellSolid(GLcoord2(world_x + 1, world_y)))
		shape |= 2;
	if (WorldCellSolid(GLcoord2(world_x + 1, world_y + 1)))
		shape |= 4;
	if (WorldCellSolid(GLcoord2(world_x, world_y + 1)))
		shape |= 8;
	_shape[local.x][local.y] = shape;
	return _shape[local.x][local.y];
}

//Clear out a specific point of the grid to make it open.
void Page::Dig(int x, int y, bool add_spawn, int size)
{
	if (x<0 || x>PAGE_EDGE)
		return;
	if (y<0 || y>PAGE_EDGE)
		return;
	for (int xx = 0; xx < size; xx++) {
		for (int yy = 0; yy < size; yy++) {
			int   safe_x = clamp(x + xx, 0, PAGE_EDGE);
			int   safe_y = clamp(y + yy, 0, PAGE_EDGE);
			_map[safe_x][safe_y] = MAP_OPEN;
		}
	}
	if (add_spawn)
		_spawn_slots.push_back(GLcoord2(x, y));
}

//The opposite of dig. Usually one loop will use dig to open up some area of
//the page, and then fill is used to add blocks back in.
void Page::Fill(int x, int y)
{
	if (x<0 || x>PAGE_EDGE)
		return;
	if (y<0 || y>PAGE_EDGE)
		return;
	_map[x][y] = MAP_SOLID;
}

void Page::FactoryDestroyed ()
{ 
	_factories--; 
	//If we're out of factories, then clear the queue, since we no longer have any way to spawn them.
	if (_factories < 1) 
		_robots.clear ();
}

void Page::Init(GLcoord2 grid_pos, int screen, int connect, std::string pattern, int doors)
{
	int       x, y;
	GLcoord2  world;

	_grid = grid_pos;
	world = _grid * PAGE_SIZE;
	_connect = (short)connect;
	_pattern = pattern;
	_screen_index = screen;
	_initialized = true;
	_origin = GLvector2((float)_grid.x * PAGE_SIZE, (float)_grid.y * PAGE_SIZE);
	_desired_doors = doors;
	_spawn_slots.clear();
	_door_info.clear();
	_robots.clear();
	_factories = 0;
	_bbox.Clear();
	_bbox.ContainPoint(world);
	_bbox.ContainPoint(GLvector2(world) + GLvector2(PAGE_SIZE, PAGE_SIZE));
	//Clear the marching squares grid.
	for (x = 0; x < PAGE_SIZE; x++)  {
		for (y = 0; y < PAGE_SIZE; y++) {
			_shape[x][y] = SHAPE_INVALID;
			_access[x][y] = false;
			_blocked[x][y] = false;
		}
	}
}

bool Page::MachineSafe(GLcoord2 local)
{
	//scan
	for (int x = -1; x <= 1; x++) {
		//The spaces overhead must be open.
		if (_map[local.x + x][local.y + 0] == MAP_SOLID)
			return false;
		//The spaces we're occupying must be open.
		if (_map[local.x + x][local.y + 1] == MAP_SOLID)
			return false;
		//The spaces under us must be solid.
		if (_map[local.x + x][local.y + 2] == MAP_OPEN)
			return false;
	}
	return true;
}

//Stick the machines just a tiny bit into the floor to hide their corner seams.
#define MACHINE_DISPLACE    0.25f

///Scan the pages and figure out where important things need to go.
void Page::DoLocations()
{
	bool    machine_done;
	//Figure out where to put a machine.
	//We want a flat area 3 spaces wide, with 2 open spaces of vertical clearance.
	//We want this point to be as high as possible.
	machine_done = false;
	//For insurance, just set a dumb broken spot right in the center of the page.
	_machine_pos = _origin + GLcoord2(PAGE_HALF, PAGE_HALF);
	for (int y = 1; y < PAGE_SIZE && !machine_done; y++) {
		for (int x = 1; x < PAGE_EDGE - 1; x++) {
			if (MachineSafe(GLcoord2(x, y))) {
				_machine_pos = _origin + GLcoord2(x, y) + GLvector2(0, MACHINE_DISPLACE);
				machine_done = true;
				break;
			}
		}
	}

	//Next we pick a spawn point.
	if (_spawn_slots.empty()) {
		_landing_pos = _origin + GLcoord2(PAGE_HALF, PAGE_EDGE - 5);
		return;
	}
	_landing_pos = _origin + _spawn_slots[0];
	return;
}

bool Page::NeedAccess(int x, int y)
{
	//Make sure this point is in bounds
	if (x < 0 || x > PAGE_EDGE)
		return false;
	if (y < 0 || y > PAGE_EDGE)
		return false;
	//Solid rock doesn't need spawn / movement access.
	if (_map[x][y] != MAP_OPEN)
		return false;
	//Don't need access if you already have it.
	if (_access[x][y])
		return false;
	return true;
}

//Perform  brute-force flood fill on this screen to identify which
//cells are reachable and which aren't.
void Page::DoAccess()
{
	GLcoord2              start;
	vector<GLcoord2>      flood;
	GLcoord2              pos;

	//First identify our starting point.
	if (_pattern == "solid")
		return;
	if (_connect & CONNECT_LEFT)
		start = GLcoord2(0, PAGE_HALF);
	else if (_connect & CONNECT_RIGHT)
		start = GLcoord2(PAGE_EDGE, PAGE_HALF);
	else if (_connect & CONNECT_UP)
		start = GLcoord2(PAGE_HALF, 0);
	else
		start = GLcoord2(PAGE_HALF, PAGE_EDGE);
	_debug_point = start;
	flood.push_back(start);
	//Flood fill.
	for (unsigned i = 0; i < flood.size(); i++) {
		pos = flood[i];
		if (_access[pos.x][pos.y] == true)
			continue;
		_access[pos.x][pos.y] = true;
		//Check all 8 ordinal neighbors.
		if (NeedAccess(pos.x, pos.y - 1))
			flood.push_back(GLcoord2(pos.x, pos.y - 1));
		if (NeedAccess(pos.x, pos.y + 1))
			flood.push_back(GLcoord2(pos.x, pos.y + 1));
		if (NeedAccess(pos.x + 1, pos.y))
			flood.push_back(GLcoord2(pos.x + 1, pos.y));
		if (NeedAccess(pos.x - 1, pos.y))
			flood.push_back(GLcoord2(pos.x - 1, pos.y));
		if (NeedAccess(pos.x - 1, pos.y - 1))
			flood.push_back(GLcoord2(pos.x - 1, pos.y - 1));
		if (NeedAccess(pos.x + 1, pos.y + 1))
			flood.push_back(GLcoord2(pos.x + 1, pos.y + 1));
		if (NeedAccess(pos.x + 1, pos.y - 1))
			flood.push_back(GLcoord2(pos.x + 1, pos.y - 1));
		if (NeedAccess(pos.x - 1, pos.y + 1))
			flood.push_back(GLcoord2(pos.x - 1, pos.y + 1));
	}
	//Examine the map, fine open areas where a machine will fit.
	for (int x = 0; x < PAGE_SIZE; x++) {
		for (int y = 0; y < PAGE_SIZE; y++) {
			_blocked[x][y] = false;
		}
	}
}

void Page::RenderDebug()
{
	for (int x = 0; x < PAGE_SIZE; x++) {
		for (int y = 0; y < PAGE_SIZE; y++) {
			if (_blocked[x][y])
				RenderQuad(_origin + GLvector2((float)x, (float)y) + GLvector2(0.5f, 0.5f), SPRITE_ALERT, GLrgba(1, 1, 0), 1.0f, 0, 0, true);
		}
	}
	GLrgba    color = GLrgbaUnique(_grid.x + _grid.y * 13);
	for (unsigned i = 0; i < _spawn_slots.size(); i++) {
		RenderQuad(_origin + _spawn_slots[i], SPRITE_TARGET, color, 0.4f, 0, 0, true);
	}
	RenderQuad(_origin + _debug_point, SPRITE_COIN, color, 4.4f, 0, 0, true);
}

//Scan the space and look for likely places to spawn enemies.
void Page::DoSpawns()
{
	unsigned    i;

	//Pass over the entire page and look for places where we could put a robot.
	for (int y = 1; y < PAGE_EDGE; y++) {
		for (int x = 1; x < PAGE_EDGE; x++) {
			if (_map[x][y] != MAP_OPEN)
				continue;
			if (_map[x - 1][y] != MAP_OPEN)
				continue;
			if (_map[x + 1][y] != MAP_OPEN)
				continue;
			if (_map[x][y + 1] != MAP_OPEN)
				continue;
			if (_map[x][y - 1] != MAP_OPEN)
				continue;
			if (!_access[x][y])
				continue;
			_spawn_slots.push_back(GLcoord2(x, y));
		}
	}
	//Go through the list and make sure they're in bounds.
	i = 0;
	while (i < _spawn_slots.size()) {
		if (_spawn_slots[i].x <= 0 || _spawn_slots[i].x > PAGE_EDGE) {
			_spawn_slots.erase(_spawn_slots.begin() + i);
			continue;
		}
		if (_spawn_slots[i].y <= 0 || _spawn_slots[i].y > PAGE_EDGE) {
			_spawn_slots.erase(_spawn_slots.begin() + i);
			continue;
		}
		i++;
	}
	//Now go through list of ALL spawns and make sure they're reachable.
	for (i = 0; i < _spawn_slots.size(); i++) {
		//If it's unreachable, kill it.
		if (!_access[_spawn_slots[i].x][_spawn_slots[i].y])
			_spawn_slots.erase(_spawn_slots.begin() + i);
	}
}

GLvector2 Page::Spawn()
{
	if (_spawn_slots.empty()) //If no spawn points, just return the center
		return _origin + GLcoord2(PAGE_HALF, PAGE_HALF);
	return _origin + _spawn_slots[RandomVal() % _spawn_slots.size()];
}

/*-----------------------------------------------------------------------------
This is used by background layers ONLY. It queries the solidity of the cell,
but uses the noise table to modify the results. Used to make the background
visually distinct from foreground.
-----------------------------------------------------------------------------*/

bool Page::CellSolidSpecial(int world_x, int world_y, float modify)
{
	GLcoord2  from_center;
	GLvector2 delta;
	float     gradient;
	float     noise;

	//See how far we are from the center of the page.
	from_center = GLcoord2(world_x % PAGE_SIZE, world_y % PAGE_SIZE);
	from_center -= PAGE_HALF;
	delta = from_center;
	//Gradient will be 1 at the center of a page and 0 at the edges.
	gradient = delta.Length() / PAGE_HALF;
	gradient *= gradient;
	gradient = 1 - min(1.0f, gradient);
	noise = Noisef(Octant(world_x, 9991, world_y));
	//Re-scale noise so it's centered around 0, from -1 to +1.
	noise = (noise*2.0f) - 1.0f;
	modify = 1.0f - modify;
	if (noise*gradient < -modify)
		return false;
	if (noise*gradient > modify)
		return true;
	return WorldCellSolid(GLcoord2(world_x, world_y));
}

bool Page::CellSolid(int world_x, int world_y, GLcoord2 radius)
{
	if (WorldCellSolid(GLcoord2(world_x, world_y - radius.y)))
		return true;
	if (WorldCellSolid(GLcoord2(world_x - radius.x, world_y)))
		return true;
	return false;
}

void Page::AddQuad(GLvector2 origin, GLuvFrame uv, GLmesh* m, float depth, float scale)
{
	int       verts;
	GLvector  v[4];
	GLvector  v2[4];

	v2[0] = GLvector2(-0.5f, -0.5f) * scale;
	v2[1] = GLvector2(0.5f, -0.5f) * scale;
	v2[2] = GLvector2(0.5f, 0.5f) * scale;
	v2[3] = GLvector2(-0.5f, 0.5f) * scale;
	for (int i = 0; i < 4; i++)
		v[i] = GLvector(origin.x + 0.5f, origin.y + 0.5f, depth) + GLvector(v2[i].x, v2[i].y, 0);
	verts = m->Vertices();
	for (int i = 0; i < 4; i++)
		m->PushVertex(v[i], uv.uv[i]);
	m->PushQuad(verts, verts + 1, verts + 2, verts + 3);
}

void Page::AddWalls(int x, int y, int shape, GLmesh* m, float depth, bool glow)
{
	GLuvFrame uv;
	GLvector  v[4];
	GLvector2 origin;
	int       variant;

	variant = _variant[x][y];
	origin = GLvector2(_origin.x + x, _origin.y + y);
	switch (shape) {
	case 0:
		break;
	case 1:
		AddQuad(origin, GetUV(variant, TILE_CEIL_SLOPE, glow), m, depth);
		break;
	case 2:
		uv = GetUV(variant, TILE_CEIL_SLOPE, glow);
		uv.Mirror();
		AddQuad(origin, uv, m, depth);
		break;
	case 3:
		uv = GetUV(variant, TILE_CEIL, glow);
		AddQuad(origin, uv, m, depth);
		break;
	case 4:
		uv = GetUV(variant, TILE_FLOOR_SLOPE, glow);
		uv.Mirror();
		AddQuad(origin, uv, m, depth);
		break;
	case 5:
		AddQuad(origin, GetUV(variant, TILE_CEIL_SLOPE, glow), m, depth);
		uv = GetUV(variant, TILE_FLOOR_SLOPE, glow);
		uv.Mirror();
		AddQuad(origin, uv, m, depth);
		break;
	case 6:
		uv = GetUV(variant, TILE_WALL, glow);
		uv.Mirror();
		AddQuad(origin, uv, m, depth);
		break;
	case 7:
		uv = GetUV(variant, TILE_CEIL_SLOPE, glow);
		uv.Mirror();
		AddQuad(origin + GLvector2(-0.5f, 0.5f), uv, m, depth);
		uv = GetUV(1, TILE_SOLID, glow);
		AddQuad(origin + GLvector2(0.0f, -0.5f), uv, m, depth);
		AddQuad(origin + GLvector2(0.5f, 0.0f), uv, m, depth);
		break;
	case 8:
		uv = GetUV(variant, TILE_FLOOR_SLOPE, glow);
		AddQuad(origin, uv, m, depth);
		break;
	case 9:
		uv = GetUV((TILE_VARIANTS - 1) - variant, TILE_WALL, glow);
		AddQuad(origin, uv, m, depth);
		break;
	case 10:
		uv = GetUV(variant, TILE_CEIL_SLOPE, glow);
		uv.Mirror();
		AddQuad(origin, uv, m, depth);
		uv = GetUV(variant, TILE_FLOOR_SLOPE, glow);
		AddQuad(origin, uv, m, depth);
		break;
	case 11:
		AddQuad(origin + GLvector2(0.5f, 0.5f), GetUV(variant, TILE_CEIL_SLOPE, glow), m, depth);
		uv = GetUV(1, TILE_SOLID, glow);
		AddQuad(origin + GLvector2(0.0f, -0.5f), uv, m, depth);
		AddQuad(origin + GLvector2(-0.5f, 0.0f), uv, m, depth);
		break;
	case 12:
		uv = GetUV(variant, TILE_FLOOR, glow);
		AddQuad(origin, uv, m, depth);
		break;
	case 13:
		AddQuad(origin + GLvector2(0.5f, -0.5f), GetUV(variant, TILE_FLOOR_SLOPE, glow), m, depth);
		uv = GetUV(0, TILE_SOLID, glow);
		AddQuad(origin + GLvector2(0.0f, 0.5f), uv, m, depth);
		AddQuad(origin + GLvector2(-0.5f, 0.0f), uv, m, depth);
		break;
	case 14:
		uv = GetUV(variant, TILE_FLOOR_SLOPE, glow);
		uv.Mirror();
		AddQuad(origin + GLvector2(-0.5f, -0.5f), uv, m, depth);
		uv = GetUV(1, TILE_SOLID, glow);
		AddQuad(origin + GLvector2(0.0f, 0.5f), uv, m, depth);
		AddQuad(origin + GLvector2(0.5f, 0.0f), uv, m, depth);
		break;
	case 15:
		AddQuad(origin, GetUV(variant, TILE_SOLID, glow), m, depth);
		break;
	}
}

void Page::BuildMesh(Zone* z)
{
	if (!_initialized)
		return;

	int       x, y;
	int       index;
	GLuvFrame uv;
	GLvector  v[4];
	bool      simple;

	for (unsigned i = 0; i < PAGE_LAYER_COUNT; i++)
		_mesh[i].Clear();
	simple = true;
	if (_pattern != "solid")
		simple = false;
	for (x = 0; x <= PAGE_SIZE && simple; x++) {
		if (!z->CellSolid(GLcoord2(_grid.x * PAGE_SIZE + PAGE_SIZE, _grid.y * PAGE_SIZE + x)))
			simple = false;
		if (!z->CellSolid(GLcoord2(_grid.x * PAGE_SIZE + x, _grid.y * PAGE_SIZE + PAGE_SIZE)))
			simple = false;
	}
	if (simple) {
		GLuvFrame   uv;
		GLbbox2     b;

		b.Clear();
		b.ContainPoint(_origin);
		b.ContainPoint(_origin + GLvector2(PAGE_SIZE, PAGE_SIZE));
		uv = GetUV(TILE_SPECIAL_BLACK, 5, false);

		_mesh[PAGE_LAYER_MAIN].PushVertex(GLvector(b.pmin.x, b.pmin.y, DEPTH_LEVEL), uv.uv[0]);
		_mesh[PAGE_LAYER_MAIN].PushVertex(GLvector(b.pmax.x, b.pmin.y, DEPTH_LEVEL), uv.uv[1]);
		_mesh[PAGE_LAYER_MAIN].PushVertex(GLvector(b.pmax.x, b.pmax.y, DEPTH_LEVEL), uv.uv[2]);
		_mesh[PAGE_LAYER_MAIN].PushVertex(GLvector(b.pmin.x, b.pmax.y, DEPTH_LEVEL), uv.uv[3]);
		_mesh[PAGE_LAYER_MAIN].PushQuad(0, 1, 2, 3);
		_mesh[PAGE_LAYER_INNER].PushVertex(GLvector(b.pmin.x, b.pmin.y, DEPTH_BG_NEAR), uv.uv[0]);
		_mesh[PAGE_LAYER_INNER].PushVertex(GLvector(b.pmax.x, b.pmin.y, DEPTH_BG_NEAR), uv.uv[1]);
		_mesh[PAGE_LAYER_INNER].PushVertex(GLvector(b.pmax.x, b.pmax.y, DEPTH_BG_NEAR), uv.uv[2]);
		_mesh[PAGE_LAYER_INNER].PushVertex(GLvector(b.pmin.x, b.pmax.y, DEPTH_BG_NEAR), uv.uv[3]);
		_mesh[PAGE_LAYER_INNER].PushQuad(0, 1, 2, 3);
		_mesh[PAGE_LAYER_OUTER].PushVertex(GLvector(b.pmin.x, b.pmin.y, DEPTH_BG_FAR), uv.uv[0]);
		_mesh[PAGE_LAYER_OUTER].PushVertex(GLvector(b.pmax.x, b.pmin.y, DEPTH_BG_FAR), uv.uv[1]);
		_mesh[PAGE_LAYER_OUTER].PushVertex(GLvector(b.pmax.x, b.pmax.y, DEPTH_BG_FAR), uv.uv[2]);
		_mesh[PAGE_LAYER_OUTER].PushVertex(GLvector(b.pmin.x, b.pmax.y, DEPTH_BG_FAR), uv.uv[3]);
		_mesh[PAGE_LAYER_OUTER].PushQuad(0, 1, 2, 3);
		return;
	}

	for (x = 0; x < PAGE_SIZE; x++) {
		for (y = 0; y < PAGE_SIZE; y++) {
			int       xp1, yp1;
			GLcoord2  corner;

			corner = _grid * PAGE_SIZE;
			xp1 = x < PAGE_EDGE ? (x + 1) : x;
			yp1 = y < PAGE_EDGE ? (y + 1) : y;
			index = 0;
			if (z->CellSolid(GLcoord2(corner.x + x, corner.y + y)))
				index |= 1;
			if (z->CellSolid(GLcoord2(corner.x + x + 1, corner.y + y)))
				index |= 2;
			if (z->CellSolid(GLcoord2(corner.x + x + 1, corner.y + y + 1)))
				index |= 4;
			if (z->CellSolid(GLcoord2(corner.x + x, corner.y + y + 1)))
				index |= 8;
			AddWalls(x, y, index, &_mesh[PAGE_LAYER_MAIN], DEPTH_LEVEL, false);
      if (EnvValueb (ENV_RENDER_OVERLAY))
			  AddWalls(x, y, index, &_mesh[PAGE_LAYER_GLOW], DEPTH_LEVEL_GLOW, true);
			if ((index & 1 || index & 2) && index != 15) {
				random_scale_index = (random_scale_index + 1) % RANDOM_SCALE_COUNT;
				AddQuad (GLvector2 (_origin.x + x, _origin.y + y), GetUV (TILE_SPECIAL_LIGHT, TILE_SPECIAL, false), &_mesh[PAGE_LAYER_GLOW], DEPTH_LEVEL - 0.01f, random_scale[random_scale_index] * 2);
			}

			index = 0;
			if (CellSolidSpecial(corner.x + x, corner.y + y, INNER_MOD))
				index |= 1;
			if (CellSolidSpecial(corner.x + x + 1, corner.y + y, INNER_MOD))
				index |= 2;
			if (CellSolidSpecial(corner.x + x + 1, corner.y + y + 1, INNER_MOD))
				index |= 4;
			if (CellSolidSpecial(corner.x + x, corner.y + y + 1, INNER_MOD))
				index |= 8;
			AddWalls(x, y, index, &_mesh[PAGE_LAYER_INNER], DEPTH_BG_NEAR, false);
			index = 0;
			if (CellSolidSpecial(corner.x + x, corner.y + y, OUTER_MOD))
				index |= 1;
			if (CellSolidSpecial(corner.x + x + 1, corner.y + y, OUTER_MOD))
				index |= 2;
			if (CellSolidSpecial(corner.x + x + 1, corner.y + y + 1, OUTER_MOD))
				index |= 4;
			if (CellSolidSpecial(corner.x + x, corner.y + y + 1, OUTER_MOD))
				index |= 8;

			if (index == 0)
				continue;
			AddWalls(x, y, index, &_mesh[PAGE_LAYER_OUTER], DEPTH_BG_FAR, false);
		}
	}
}

void Page::RobotsPush(int id)
{
	_robots.push_back(id);
}

int Page::RobotsPop()
{
	int id;

	if (_robots.empty())
		return ROBOT_INVALID;
	id = _robots[_robots.size() - 1];
	_robots.pop_back();
	return id;
}

void Page::RobotsRandomize()
{
	int   other;
	int   temp;

	for (unsigned i = 0; i < _robots.size(); i++) {
		other = RandomVal(_robots.size());
		temp = _robots[i];
		_robots[i] = _robots[other];
		_robots[other] = temp;
	}
}