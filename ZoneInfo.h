#pragma once

#include "fx.h"

struct ZoneMob
{
	int     type_index;
	int     count_min;
	int     count_max;
	bool    from_machine;

	//If this is <= 0 or not specified, this mob can be in any room.
	//If this is a valid room number, this mob can only be in that room and no other mobs will be in that room unless specified
	//This is to stop other robots from appearing in our scripted room
	int     room_index;
};

struct Motif
{
	string                _texture_fore;
	string                _texture_mid;
	string                _texture_back;
	string                _texture_sky;
	GLrgba                _color[COLOR_COUNT];
	float                 _fog;
	vector<string>        _machines;
	int                   _wall_damage;
	bool									_blind;

	void                  Init(void* mnode_ptr);
};

//The zone info that does not need to be saved to file
struct ZoneInfo
{
	//These two are identifiers for a zone
	//(think database key in SQL)

	int                   _map_id;
	int                   _zone_id;

	//The rest are parameters associated with a zone

	int                   _length;
	bool                  _is_combat;
	bool                  _has_motif;     ///If true, then this zone controls its motif and ignores the one offered by the map.
	struct Motif          _motif;
	vector<std::string>   _patterns;
	std::string           _music;
	std::string           _end_machine;
	vector<ZoneMob>       _mobs;
	SpriteEntry           _door_sprite;
	enum ForcefieldType		_forcefield;
};

//The zone info that needs to be saved along with player state
struct PlayerZoneInfo
{
	//These two are identifiers for a zone
	//(think database key in SQL)

	int                   _map_id;
	int                   _zone_id;

	//The rest are parameters associated with a zone

	bool                  _is_complete;
};
