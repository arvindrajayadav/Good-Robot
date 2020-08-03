#ifndef PAGE_H
#define PAGE_H

#include "fx.h" //for DoorFacing
#include "fxmachine.h" //for MachineMount
#include "TMXMap.h"  //for Doomhammer! (TMXMap)

#define CONNECT_NONE    0
#define CONNECT_UP      1
#define CONNECT_DOWN    2
#define CONNECT_LEFT    4
#define CONNECT_RIGHT   8

#define DOORS_UP        16
#define DOORS_DOWN      32
#define DOORS_LEFT      64
#define DOORS_RIGHT     128

#define LAYER_INNER     1
#define LAYER_OUTER     2
#define LAYER_PLAY      4

#define SHAPE_INVALID   16

enum ePageLayer
{
	PAGE_LAYER_OUTER,
	PAGE_LAYER_INNER,
	PAGE_LAYER_MAIN,
	PAGE_LAYER_GLOW,
	PAGE_LAYER_DEBUG,
	PAGE_LAYER_COUNT,
};

enum
{
	UV_Z,
	UV_EDGE,
};

enum
{
	MAP_OPEN,
	MAP_SOLID,
};

struct MarchMap
{
	unsigned char map[PAGE_SIZE][PAGE_SIZE];
};

struct DoorInfo
{
	GLvector2       position;
	enum DoorFacing facing;
};

class Page
{
	GLcoord2          _grid;                            //The origin of the NW corner, whole number
	GLvector2         _origin;                          //The origin of the NW corner of screen as vector
	GLvector2         _landing_pos;                     //Where the checkpoint will be placed, if present.
	GLvector2         _machine_pos;                     //Where to put a machine on this page.
	GLbbox2           _bbox;                            //The bounding rectangle that contains this room.
	unsigned char     _map[PAGE_SIZE][PAGE_SIZE];       //Keeps track of the contents of each cell, solid / nonsolid.

	short             _shape[PAGE_SIZE][PAGE_SIZE];       //Keeps track of marching square patterns, for collision
	unsigned char     _variant[PAGE_SIZE][PAGE_SIZE];     //Keeps track of which tile variation is used for this cell.
	bool              _access[PAGE_SIZE][PAGE_SIZE];    //True if this spot can be pathed to.
	bool              _blocked[PAGE_SIZE][PAGE_SIZE];   ///True if this spot is open, and no machine has been placed here.
	bool              _initialized;
	short             _connect;                         //How this screen connects to neighbors
	std::string       _pattern;                         //What kind of shapes to use for the playspace
	int               _screen_index;                    //Pages are numbered incrementally through a level.
	int               _desired_doors;
	int								_factories;												//How many factories we have in this room.
	GLmesh            _mesh[PAGE_LAYER_COUNT];
	vector<GLcoord2>  _spawn_slots;
	vector<DoorInfo>  _door_info;
	vector<int>       _robots;                          //The id's of robots that can be spawned here.
	GLcoord2          _debug_point;

	bool              CellSolidSpecial(int world_x, int world_y, float modify);
	bool              CellSolid(int world_x, int world_y, GLcoord2 radius);
	void              AddWalls(int x, int y, int shape, GLmesh* m, float depth, bool glow);
	void              AddQuad(GLvector2 origin, GLuvFrame uv, GLmesh* m, float depth, float scale = 1);
	void              DoDoors(int doors, pyrodactyl::TMXMap &tmx);
	void              DoSpawns();
	void              DoAccess();
	void              DoLocations();
	void              Dig(int x, int y, bool add_spawn = false, int size = 1);
	void              Fill(int x, int y);
	GLuvFrame         GetUV(int vary, int shape, bool glow);
	bool              NeedAccess(int x, int y);
	bool              MachineSafe(GLcoord2 local);

	bool              AreaScan(GLcoord2 start, GLcoord2 end, int desired_shape);

public:
	Page() { _initialized = false; }
	void              BuildPattern();
	void              BuildMesh(class Zone* owner);
	bool              Contains(GLvector2 p) const { return _bbox.Contains(p); }
	vector<DoorInfo>  DoorList() { return _door_info; };
	void              Init(GLcoord2 grid_pos, int screen, int connect, std::string pattern, int doors);
	GLvector2         Landing() { return _landing_pos; }
	GLvector2         Machine() { return _machine_pos; }
	GLcoord2          MachineLocation(enum MachineMount m, GLcoord2 size);
	GLmesh            Mesh(ePageLayer l) const { return _mesh[l]; }
	int               PageNumber() { return _screen_index; };
	const char*       Pattern() const { return _pattern.c_str (); }

	void              RenderDebug();
	int               RobotsCount() const { return _robots.size(); }
	void              RobotsPush(int id);
	int               RobotsPop();
	void              RobotsRandomize();

	void							FactoryAdded () { _factories++; };
	void							FactoryDestroyed ();

	bool              Solid(int local_x, int local_y) { return _map[local_x][local_y] == MAP_SOLID; }
	GLvector2         Spawn();
	short             Shape(int world_x, int world_y);
};

#endif // PAGE_H
