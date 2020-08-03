#ifndef ZONE_H
#define ZONE_H

#include "map.h"
#include "page.h"

#define MAX_ZONE_SIZE       15
#define ZONE_NEXT_LEVEL     -1

struct ZoneExitDoor
{
	enum SpriteEntry          sprite;
	int                       zone_id;
};

class Zone
{
	GLvector2                 _entry;
	GLvector2                 _respawn;
	class Page                _page[MAX_ZONE_SIZE][MAX_ZONE_SIZE];
	GLbbox2                   _bbox;
	GLcoord2                  _grid_min;
	GLcoord2                  _grid_max;
	GLcoord2                  _enter_page;
	GLcoord2                  _exit_page;
	GLcoord2                  _cell_size;
	vector<GLcoord2>          _path;                      //The coords of each page that forms our linear tunnel.
	vector<ZoneExitDoor>      _exits;
	vector<string>            _machines;
	GLrgba                    _color_layer[COLOR_COUNT];
	float                     _fog;
	GLmesh										_mesh[PAGE_LAYER_COUNT];
	VBO                       _vbo[PAGE_LAYER_COUNT];
	struct ZoneInfo           _zone_info;
  int                       _wall_damage;
	bool											_blind;
	bool											_spawners_empty;

	GLbbox2                   _sky_box;
	GLvector2                 _sky_uv;

	void											SpawnersCheck ();
	int                       Connection(int index);
	///Returns the INSIDE spot beside the door.
	GLvector2                 DoorLanding(GLvector2 position, DoorFacing direction);
	std::string               Pattern(int index);
	bool                      PlaceMachine(GLcoord2 page, string name, GLvector2& location);

public:
	void                      Activate(bool final_zone);
	bool											Blind () { return _blind; }
	GLbbox2                   Bounds() const { return _bbox; }
	GLrgba                    Color(enum MapColor c) { return _color_layer[c]; }
	void											Compile ();
	bool											SpawnersEmpty () { return _spawners_empty; }
	float                     Fog () { return _fog; }
	const Motif*              Init (struct ZoneInfo* zone, const struct Motif* motif, vector<ZoneExitDoor> exits);
  Page*											PageGet (GLcoord2 p) { return &_page[p.x][p.y];  }
	void                      Render(ePageLayer layer, unsigned texture_id);
	void                      RenderSky();
	int                       RobotSpawnId(GLcoord2 page);
	int                       RobotSpawnCount(GLcoord2 page) const;
	int                       RobotSpawnCount (int room) const;
	int                       RoomFromPosition(GLvector2) const;
	int                       RoomCount() const { return _path.size(); }
	GLvector2                 RoomPosition(int room) const;
	bool                      CellSolid(GLcoord2 pos);
	short                     CellShape(GLcoord2 pos);
	GLvector2                 Entry() { return _entry; }
	GLvector2                 Respawn() { return _respawn; }
  int                       WallDamage () { return _wall_damage; };
};

#endif // ZONE_H