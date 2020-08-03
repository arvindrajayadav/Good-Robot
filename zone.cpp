/*-----------------------------------------------------------------------------

Zone.cpp

This keeps track of the short multi-room sections within levels.

Good Robot
(c) 2015 Pyrodactyl

-----------------------------------------------------------------------------*/

#include "master.h"

#include "env.h"
#include "entity.h"
#include "fxmachine.h"
#include "map.h"
#include "page.h"
#include "player.h"
#include "random.h"
#include "world.h"
#include "zone.h"

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

class ZonePath
{
	vector<GLcoord2>  _path;
	GLcoord2          _min;
	GLcoord2          _max;
	GLcoord2          _current;

	bool              IsValid(GLcoord2 consider);
	GLcoord2          Direction(int d);
	void              Push(GLcoord2);
	vector<GLcoord2>  Panic(int length);
public:
	vector<GLcoord2>  Plot(int length);
};

struct SpecialZoneIndex
{
	int mob_index;
	int room_index;
};

bool IsSpecialRoom(std::vector<SpecialZoneIndex> &special, const int &room_index)
{
	if (!special.empty())
	{
		for (auto i : special)
			if (i.room_index == room_index)
				return true;
	}

	return false;
}

bool IsSpecialMob(std::vector<SpecialZoneIndex> &special, const int &mob_index)
{
	if (!special.empty())
	{
		for (auto i : special)
			if (i.mob_index == mob_index)
				return true;
	}

	return false;
}

bool ZonePath::IsValid(GLcoord2 t)
{
	//Make sure this position is touching at least ONE edge of our bounding box,
	//or else this room could end up spiraling in and trapping itself.
	if (t.x != _min.x && t.x != _max.x && t.y != _min.y && t.y != _max.y)
		return false;
	//Make sure we're not going to land on an existing spot
	for (unsigned i = 0; i < _path.size(); i++) {
		if (_path[i] == t)
			return false;
	}
	return true;
}

void ZonePath::Push(GLcoord2 n)
{
	_path.push_back(n);
	_min.x = min(_min.x, n.x);
	_min.y = min(_min.y, n.y);
	_max.x = max(_max.x, n.x);
	_max.y = max(_max.y, n.y);
	_current = n;
}

GLcoord2 ZonePath::Direction(int d)
{
	d %= 4;
	if (d < 0)
		d = 4 + d;
	if (d == 0) return GLcoord2(0, -1);//Up
	if (d == 1) return GLcoord2(1, 0);//Right
	if (d == 2) return GLcoord2(0, 1);//Down
	return GLcoord2(-1, 0);//Left
}

vector<GLcoord2> ZonePath::Panic(int length)
{
	vector<GLcoord2>  path;

	for (unsigned i = 0; i < length; i++) {
		path.push_back(GLcoord2(i, 0));
	}
	return path;
}

vector<GLcoord2> ZonePath::Plot(int length)
{
	int   dir;
	int   fails;
	bool  turn_right;

	_max = _min = GLcoord2(0, 0);
	Push(GLcoord2(0, 0));
	dir = RandomVal(4);
	turn_right = RandomVal(2) == 0;
	fails = 0;

	while (_path.size() < length) {
		GLcoord2  consider;

		consider = _current + Direction(dir);
		if (IsValid(consider)) {
			Push(consider);
			fails = 0;
			turn_right = !turn_right;
			dir = RandomVal(4);
		}
		else {
			dir += turn_right ? 1 : -1;
			fails++;
		}
		//We might spiral in and get stuck somewhere where this IS no good move...
		if (fails > 4)
			return Panic(length);
	}
	//Normalize the grid so all map points are in positive numbers.
	for (unsigned i = 0; i < _path.size(); i++) {
		_path[i].x -= _min.x;
		_path[i].y -= _min.y;
	}
	return _path;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

std::string Zone::Pattern(int index)
{
	std::string result;

	if (index == 0 || index == _path.size() - 1)
		result = "doors";
	else
		result = _zone_info._patterns[RandomVal(_zone_info._patterns.size())];

	return result;
}

int Zone::Connection(int index)
{
	int   result;

	result = CONNECT_NONE;
	//return result;
	if (index > 0) {
		if (_path[index - 1].x < _path[index].x)
			result |= CONNECT_LEFT;
		if (_path[index - 1].x > _path[index].x)
			result |= CONNECT_RIGHT;
		if (_path[index - 1].y > _path[index].y)
			result |= CONNECT_DOWN;
		if (_path[index - 1].y < _path[index].y)
			result |= CONNECT_UP;
	}
	if (index < _path.size() - 1) {
		if (_path[index + 1].x < _path[index].x)
			result |= CONNECT_LEFT;
		if (_path[index + 1].x > _path[index].x)
			result |= CONNECT_RIGHT;
		if (_path[index + 1].y > _path[index].y)
			result |= CONNECT_DOWN;
		if (_path[index + 1].y < _path[index].y)
			result |= CONNECT_UP;
	}
	if (_path[index].x == 0)
		result |= DOORS_LEFT;
	if (_path[index].x == _grid_max.x)
		result |= DOORS_RIGHT;
	if (_path[index].y == 0)
		result |= DOORS_UP;
	if (_path[index].y == _grid_max.y)
		result |= DOORS_DOWN;
	return result;
}

GLvector2 Zone::RoomPosition(int room) const
{
	GLcoord2    local;

	local = _path[room];
	return GLvector2((float)local.x * PAGE_SIZE, (float)local.y  * PAGE_SIZE) + GLvector2(PAGE_HALF, PAGE_HALF);
}

const Motif* Zone::Init(ZoneInfo* zi, const struct Motif* motif_ptr, vector<ZoneExitDoor> exits)
{
	//	GLmesh              temp_mesh[PAGE_LAYER_COUNT];
	GLcoord2            local;
	ZonePath            zp;
	const struct Motif* motif;

	_bbox.Clear();
	_path.clear();
	_zone_info = *zi;
	_exits = exits;
	for (int l = 0; l < PAGE_LAYER_COUNT; l++)
		_mesh[l].Clear();

	PlayerZoneInfo pzi;
	pzi._map_id = _zone_info._map_id;
	pzi._zone_id = _zone_info._zone_id;
	pzi._is_complete = true;
	Player()->ZoneAdd(pzi, true);

	//Does this zone override the given motif?
	if (zi->_has_motif)
		motif = &zi->_motif;
	else //Nope, just use the one given.
		motif = motif_ptr;

	_grid_min = GLcoord2(MAX_ZONE_SIZE, MAX_ZONE_SIZE);
	_grid_max = GLcoord2(0, 0);
	_path.push_back(GLcoord2(1, 0));
	_path.push_back(GLcoord2(0, 0));
	_path.push_back(GLcoord2(0, 1));
	_path.push_back(GLcoord2(1, 1));
	_path.push_back(GLcoord2(2, 1));
	_path = zp.Plot(zi->_length);
	//Set up the bounding rectangle and uv values for the background image.
	_sky_box.Clear();
	_sky_box.ContainPoint(GLvector2(-MAX_ZONE_SIZE, -MAX_ZONE_SIZE)*PAGE_SIZE);
	_sky_box.ContainPoint(GLvector2(MAX_ZONE_SIZE + 1, MAX_ZONE_SIZE + 1)*PAGE_SIZE);
	_sky_uv.x = MAX_ZONE_SIZE * 2 + 1;
	_sky_uv.y = MAX_ZONE_SIZE * 2 + 1;
	_wall_damage = motif->_wall_damage;
	_blind = motif->_blind;
	//Pull our colors out of the given motif.
	for (int i = 0; i < COLOR_COUNT; i++) {
		_color_layer[i] = motif->_color[i];
	}
	_fog = motif->_fog;
	_machines = motif->_machines;
	//Clear the grid.
	for (int y = 0; y < MAX_ZONE_SIZE; y++) {
		for (int x = 0; x < MAX_ZONE_SIZE; x++) {
			_page[x][y].Init(GLcoord2(x, y), -1, 0, "invalid", 0);
		}
	}
	//Inventory the screens and figure out the dimensions of our gamespace.
	for (unsigned i = 0; i < _path.size(); i++) {
		local = _path[i];
		//Keep track of how much of our grid we're using.
		_grid_min.x = min(_grid_min.x, local.x);
		_grid_min.y = min(_grid_min.y, local.y);
		_grid_max.x = max(_grid_max.x, local.x);
		_grid_max.y = max(_grid_max.y, local.y);
		_cell_size.x = _grid_max.x * PAGE_SIZE + PAGE_SIZE;
		_cell_size.y = _grid_max.y * PAGE_SIZE + PAGE_SIZE;
	}
	//Now place the gameplay screens, replacing the solid mass we just made.
	for (unsigned i = 0; i < _path.size(); i++) {
		int doors;

		local = _path[i];
		doors = 0;
		//The first room will have a locked door for the "entrance".
		if (i == 0) {
			_enter_page = local;
			doors = 1;
		}
		//The last room will have all the exits requested when we were initialized.
		if (i == _path.size() - 1) {
			_exit_page = local;
			doors = _exits.size();
		}
		//Fill this page with data.
		_page[local.x][local.y].Init(local, i, Connection(i), Pattern(i), doors);
		_bbox.ContainPoint(GLvector2(1, 1) + local*PAGE_SIZE);
		_bbox.ContainPoint(GLvector2(1, 1) + GLvector2(local*PAGE_SIZE) + GLvector2(PAGE_SIZE - 2, PAGE_SIZE - 2));
	}
	//Procedurally generate each screen to fill in our grid of marching squares.
	for (int y = _grid_min.y; y <= _grid_max.y; y++) {
		for (int x = _grid_min.x; x <= _grid_max.x; x++) {
			_page[x][y].BuildPattern();
		}
	}
	//Build the mesh for each page, and add that mesh to the zone mesh.
	for (int l = 0; l < PAGE_LAYER_COUNT; l++)
		_mesh[l].Clear();
	for (int y = _grid_min.y; y <= _grid_max.y; y++) {
		for (int x = _grid_min.x; x <= _grid_max.x; x++) {
			_page[x][y].BuildMesh(this);
			for (int l = 0; l < PAGE_LAYER_COUNT; l++)
				_mesh[l] += _page[x][y].Mesh((ePageLayer)l);
		}
	}
	//Add a buffer of blank pages on the top and bottom edge of the zone.
	Page    p;

	for (int x = _grid_min.x - 1; x <= _grid_max.x + 1; x++) {
		p.Init(GLcoord2(x, _grid_min.y - 1), -1, 0, "solid", 0);
		p.BuildPattern();
		p.BuildMesh(this);
		for (int l = 0; l < PAGE_LAYER_COUNT; l++)
			_mesh[l] += p.Mesh((ePageLayer)l);
		p.Init(GLcoord2(x, _grid_max.y + 1), -1, 0, "solid", 0);
		p.BuildPattern();
		p.BuildMesh(this);
		for (int l = 0; l < PAGE_LAYER_COUNT; l++)
			_mesh[l] += p.Mesh((ePageLayer)l);
	}
	//Add a buffer of blank pages on the left and right edge of the zone.
	for (int y = _grid_min.y; y <= _grid_max.y; y++) {
		p.Init(GLcoord2(_grid_min.x - 1, y), -1, 0, "solid", 0);
		p.BuildPattern();
		p.BuildMesh(this);
		for (int l = 0; l < PAGE_LAYER_COUNT; l++)
			_mesh[l] += p.Mesh((ePageLayer)l);
		p.Init(GLcoord2(_grid_max.x + 1, y), -1, 0, "solid", 0);
		p.BuildPattern();
		p.BuildMesh(this);
		for (int l = 0; l < PAGE_LAYER_COUNT; l++)
			_mesh[l] += p.Mesh((ePageLayer)l);
	}
	Compile();
	return motif;
}

void Zone::Compile()
{
	//Compile the meshes into a vertex buffer.
	for (int l = 0; l < PAGE_LAYER_COUNT; l++)
		_vbo[l].Create(&_mesh[l]);
}

void Zone::RenderSky()
{
	GLrgba    shadow;
	GLrgba    color_sky;

	color_sky = Color(COLOR_SKY);
	shadow = color_sky * Fog();
	_sky_uv.x = MAX_ZONE_SIZE * 2 + 1;
	_sky_uv.y = MAX_ZONE_SIZE * 2 + 1;
	//Draw the entire sky, darkened.
	glColor3fv(&shadow.red);
	glDisable(GL_STENCIL_TEST);
	glBegin(GL_QUADS);
	glTexCoord2f(0, _sky_uv.y);          glVertex3f(_sky_box.pmin.x, _sky_box.pmin.y, DEPTH_SKY);
	glTexCoord2f(_sky_uv.x, _sky_uv.y);  glVertex3f(_sky_box.pmax.x, _sky_box.pmin.y, DEPTH_SKY);
	glTexCoord2f(_sky_uv.x, 0);          glVertex3f(_sky_box.pmax.x, _sky_box.pmax.y, DEPTH_SKY);
	glTexCoord2f(0, 0);                  glVertex3f(_sky_box.pmin.x, _sky_box.pmax.y, DEPTH_SKY);
	glEnd();
	//Mask out the parts the player can't see and draw it again in full brightness.
	if (EnvValueb(ENV_SHADOWS))
		glEnable(GL_STENCIL_TEST);
	glColor3fv(&color_sky.red);
	glBegin(GL_QUADS);
	glTexCoord2f(0, _sky_uv.y);          glVertex3f(_sky_box.pmin.x, _sky_box.pmin.y, DEPTH_SKY);
	glTexCoord2f(_sky_uv.x, _sky_uv.y);  glVertex3f(_sky_box.pmax.x, _sky_box.pmin.y, DEPTH_SKY);
	glTexCoord2f(_sky_uv.x, 0);          glVertex3f(_sky_box.pmax.x, _sky_box.pmax.y, DEPTH_SKY);
	glTexCoord2f(0, 0);                  glVertex3f(_sky_box.pmin.x, _sky_box.pmax.y, DEPTH_SKY);
	glEnd();
}

void Zone::Render(ePageLayer layer, unsigned texture)
{
	GLrgba    color;
	GLrgba    shadow;
	int       color_index;

	glDisable(GL_STENCIL_TEST);
	glBindTexture(GL_TEXTURE_2D, texture);
	if (layer == PAGE_LAYER_OUTER && !EnvValueb(ENV_RENDER_BACKGROUND))
		return;
	if (layer == PAGE_LAYER_INNER && !EnvValueb(ENV_RENDER_BACKGROUND))
		return;
	if (layer == PAGE_LAYER_MAIN && !EnvValueb(ENV_RENDER_WALLS))
		return;
	if (layer == PAGE_LAYER_GLOW && !EnvValueb(ENV_RENDER_GLOW))
		return;

	if (layer == PAGE_LAYER_DEBUG) {
		if (!EnvValueb(ENV_BBOX))
			return;
		for (unsigned room = 0; room < _path.size(); room += 1) {
			GLcoord2            local = _path[room];
			_page[local.x][local.y].RenderDebug();
		}
		glColor3f(1, 1, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		if (EnvValueb(ENV_BBOX))
			_bbox.Render();
		return;
	}
	if (layer == PAGE_LAYER_OUTER)
		color_index = COLOR_BACKGROUND2;
	else if (layer == PAGE_LAYER_INNER)
		color_index = COLOR_BACKGROUND1;
	else if (layer == PAGE_LAYER_GLOW)
		color_index = COLOR_LIGHT;
	else
		color_index = COLOR_FOREGROUND;

	color = _color_layer[color_index];
	shadow = color * _fog;
	glColor3fv(&shadow.red);
	_vbo[layer].Render();
	if (layer != PAGE_LAYER_MAIN)
		glEnable(GL_STENCIL_TEST);
	glColor3fv(&color.red);
	_vbo[layer].Render();
}

bool Zone::CellSolid(GLcoord2 world)
{
	GLcoord2  local;
	int       row, column;

	if (world.x < 1 || world.y < 1 || world.x >= _cell_size.x || world.y >= _cell_size.y)
		return true;
	world.x = clamp(world.x, 0, (MAX_ZONE_SIZE * PAGE_SIZE) - 1);
	world.y = clamp(world.y, 0, (MAX_ZONE_SIZE * PAGE_SIZE) - 1);
	local.x = world.x % PAGE_SIZE;
	local.y = world.y % PAGE_SIZE;
	row = (int)world.y / PAGE_SIZE;
	column = (int)world.x / PAGE_SIZE;
	return _page[column][row].Solid(local.x, local.y);
}

short Zone::CellShape(GLcoord2 world)
{
	int       row, column;

	if (world.x < 0)
		return 9;
	if (world.y < 0)
		return 3;
	if (world.x >= _cell_size.x)
		return 6;
	if (world.y >= _cell_size.y)
		return 12;
	world.x = clamp(world.x, 0, (MAX_ZONE_SIZE * PAGE_SIZE) - 1);
	world.y = clamp(world.y, 0, (MAX_ZONE_SIZE * PAGE_SIZE) - 1);
	row = (int)world.y / PAGE_SIZE;
	column = (int)world.x / PAGE_SIZE;
	return _page[column][row].Shape(world.x, world.y);
}

bool Zone::PlaceMachine(GLcoord2 page, string name, GLvector2& location)
{
	fxMachine*          m;
	const MachineInfo*  m_info;
	GLcoord2            cell;
	GLvector2           center;

	//Get the info for this machine.
	m_info = EnvMachineFromName(name);
	//Make sure the machine exists.
	if (m_info == NULL)
		return false;
	//Find a spot in this page where this machine can be placed.
	cell = _page[page.x][page.y].MachineLocation(m_info->Mount(), m_info->Size());
	//If we didn't find a suitable mount location...
	if (cell == GLcoord2())
		return false;
	center.x = (float)cell.x + 0.5f;
	center.y = (float)cell.y + 0.5f;
	//It's all good. Drop it into the gameworld.
	m = new fxMachine;
	m->Init(center, page, m_info);
	EntityDeviceAdd(m);
	location = center;
	return true;
}

GLvector2 Zone::DoorLanding(GLvector2 position, DoorFacing direction)
{
	if (direction == DOOR_DOWN)
		position.y += 1.0f;
	else if (direction == DOOR_UP)
		position.y -= 1.0f;
	else if (direction == DOOR_LEFT)
		position.x -= 1.0f;
	else if (direction == DOOR_RIGHT)
		position.x += 1.0f;
	return position;
}

void Zone::Activate(bool final_zone)
{
	vector<DoorInfo>    door_list;
	fxDoor*							d;
	Page*								p;

	//Add the fake entrance door.
	p = &_page[_enter_page.x][_enter_page.y];
	door_list = p->DoorList();
	d = new fxDoor;
	d->Init(door_list[0].position, door_list[0].facing, SPRITE_DOOR_LOCKED, 0, true);
	EntityDeviceAdd(d);
	//First door is our entry point.
	_entry = DoorLanding(door_list[0].position, door_list[0].facing);
	//Add the exits.
	p = &_page[_exit_page.x][_exit_page.y];
	door_list = p->DoorList();
	//These two arrays SHOULD be the same size...
	for (unsigned i = 0; i < door_list.size(); i++) {
		d = new fxDoor;
		d->Init(door_list[i].position, door_list[i].facing, _exits[i].sprite, _exits[i].zone_id, false);
		EntityDeviceAdd(d);
	}
	//Place the machines.
	vector<bool>      room_has_factory;

	for (unsigned room = 0; room < _path.size(); room++) {
		GLcoord2            local = _path[room];
		vector<string>      machine_pool;
		string              chosen;
		const MachineInfo*  m_info;
		bool                success;
		int                 count;
		int                 rand_index;
		bool                has_factory = false;
		bool                factory_forbidden = false;
		Page*               p;
		GLvector2           ignore;

		p = &_page[local.x][local.y];
		//First room gets the respawn station and hat machine.
		if (room == 0) {
			PlaceMachine(local, "Spawner", _respawn);
			PlaceMachine(local, "Hats", ignore);
			factory_forbidden = true;
		}
		//If this is the last room...
		if (room == _path.size() - 1) {
			factory_forbidden = true;
			//In the last zone of a level, you get both machines.
			if (final_zone) {
				PlaceMachine(local, "Shop", ignore);
				PlaceMachine(local, "Upgrade", ignore);
			}	else //Get whatever machine is specified in the zone info.
				PlaceMachine(local, _zone_info._end_machine, ignore);

			//Last room is also protected by boss-forcefield.
			fxForcefield*   ff;

			ff = new fxForcefield();
			ff->Init(GLvector2((float)local.x, (float)local.y) * PAGE_SIZE, _zone_info._forcefield);
			EntityFxAdd(ff);
		}

		count = 0;
		machine_pool = _machines;
		//Remove factories from first / last room.
		if (factory_forbidden) {
			for (int i = 0; i < machine_pool.size(); i++) {
				m_info = EnvMachineFromName(machine_pool[i]);
				if (m_info->Use() == MACHINE_USE_FACTORY) {
					machine_pool.erase(machine_pool.begin() + i);
					i--;
				}
			}
		}
		while (count < 10 && !machine_pool.empty()) {
			rand_index = RandomVal(machine_pool.size());
			chosen = machine_pool[rand_index];
			success = PlaceMachine(local, chosen, ignore);
			//Get the info for this machine.
			m_info = EnvMachineFromName(chosen);
			//If we couldn't find a spot for this machine, take it out of the list.
			if (!success)
				machine_pool.erase(machine_pool.begin() + rand_index);
			if (success && m_info->Use() == MACHINE_USE_FACTORY)
				has_factory = true;
		}
		room_has_factory.push_back(has_factory);
	}

	//First, find which rooms are "special" rooms that do not have normal enemies in them
	std::vector<SpecialZoneIndex> special;

	//Make sure there are no invalid values in the list
	for (unsigned i = 0; i < _zone_info._mobs.size(); ++i) {
		if (_zone_info._mobs[i].room_index > 0 && _zone_info._mobs[i].room_index < _path.size()) {
			SpecialZoneIndex s;
			s.mob_index = i;
			s.room_index = _zone_info._mobs[i].room_index;
			special.push_back(s);
		}
	}

	//Add robots to non-special rooms
	for (unsigned room = 1; room < _path.size() - 1; room++)
		if (!IsSpecialRoom(special, room)) {
			GLcoord2  local = _path[room];
			Robot     b;
			int       how_many;

			//Only add non-special mobs to these rooms
			for (unsigned m = 0; m < _zone_info._mobs.size(); m++)
				if (!IsSpecialMob(special, m)) {
					how_many = RandomVal(_zone_info._mobs[m].count_max);
					how_many = max(how_many, _zone_info._mobs[m].count_min);
					//Make sure the types are valid. Artist may have specified bogus name in levels file.
					if (_zone_info._mobs[m].type_index <= 0)
						continue;
					for (unsigned i = 0; i < how_many; i++) {
						//If the room has a factory, then stick the bots into the spawn queue for
						//the factory to "create" later. If not, then spawn the bots right now.
						if (room_has_factory[room] && _zone_info._mobs[m].from_machine)
							_page[local.x][local.y].RobotsPush(_zone_info._mobs[m].type_index);
						else {
							b.Init(_page[local.x][local.y].Spawn(), _zone_info._mobs[m].type_index);
							EntityRobotAdd(b);
						}
					}
				}
			_page[local.x][local.y].RobotsRandomize();
		}

	//Add the robots to special rooms
	for (auto s : special)	{
		int       m = s.mob_index;
		int       room = s.room_index;

		GLcoord2  local = _path[room];
		Robot     b;
		int       how_many;

		how_many = RandomVal(_zone_info._mobs[m].count_max);
		how_many = max(how_many, _zone_info._mobs[m].count_min);
		//Make sure the types are valid. Artist may have specified bogus name in levels file.
		if (_zone_info._mobs[m].type_index <= 0)
			continue;
		for (unsigned i = 0; i < how_many; i++) {
			//If the room has a factory, then stick the bots into the spawn queue for
			//the factory to "create" later. If not, then spawn the bots right now.
			if (room_has_factory[room] && _zone_info._mobs[m].from_machine)
				_page[local.x][local.y].RobotsPush(_zone_info._mobs[m].type_index);
			else {
				b.Init(_page[local.x][local.y].Spawn(), _zone_info._mobs[m].type_index);
				EntityRobotAdd(b);
			}
		}

		_page[local.x][local.y].RobotsRandomize();
	}
	SpawnersCheck ();
	//And done.
	PlayerSpawn(_entry);
}

void Zone::SpawnersCheck ()
{
	_spawners_empty = true;
	for (unsigned room = 1; room < _path.size () - 1; room++) {
		GLcoord2  local = _path[room];
		if (_page[local.x][local.y].RobotsCount ()) {
			_spawners_empty = false;
			return;
		}
	}
}

int Zone::RobotSpawnId(GLcoord2 page)
{
	int		robot_id = _page[page.x][page.y].RobotsPop ();

	//See if we're out of robots to spawn.
	SpawnersCheck ();
	return robot_id;
}

int Zone::RobotSpawnCount(GLcoord2 page) const
{
	return _page[page.x][page.y].RobotsCount();
}

int Zone::RobotSpawnCount(int room) const
{
	GLcoord2 page = _path[room];
	return _page[page.x][page.y].RobotsCount();
}

int Zone::RoomFromPosition(GLvector2 pos) const
{
	for (unsigned room = 1; room < _path.size(); room++) {
		GLcoord2    local = _path[room];
		if (_page[local.x][local.y].Contains(pos))
			return room;
	}
	return 0;
}