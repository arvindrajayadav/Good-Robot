#ifndef MAP_H
#define MAP_H

#include "fx.h"
#include "robot.h"
#include "ZoneInfo.h"

#define LEVEL_NONE            -1
#define LEVEL_INTERMISSION    -2


enum MapTexture
{
	TEXTURE_FRONT,
	TEXTURE_MIDDLE,
	TEXTURE_BACK,
	TEXTURE_SKY
};

struct MapInfo
{
	string                title;
	string                subtitle;
	string                texture_fore;
	string                texture_mid;
	string                texture_back;
	string                texture_sky;
	GLrgba                color_layer[COLOR_COUNT];
	bool                  valid;
	bool                  ready;
	float                 fog_of_war;
	int                   swarm_size;
	string                music_track;
	vector<ZoneInfo>      zones;
};

class Map
{
	string                _title;
	string                _subtitle;
	string                _music;
	string                _texture_fore;
	string                _texture_mid;
	string                _texture_back;
	string                _texture_sky;
	float                 _fog;

	GLrgba                _color[COLOR_COUNT];
	vector<ZoneInfo>      _zones;
	vector<Motif>         _motifs;
  int                   _min_zones;   //How many zones the player must complete before they can face the boss.
  int                   _max_zones;   //How many zones the player can complete before they are FORCED to face the boss.

public:
	void                  Init(int index);
  const char*           Music (int zone_num);
	vector<ZoneInfo>*     Zones() { return &_zones; }
  int                   ZonesMin () { return _min_zones; }
  int                   ZonesMax () { return _max_zones; }
  unsigned              ZoneCount () const {   return _zones.size ();  }
	string                TextureName(MapTexture t);
	string                Title() { return _title; }
	string                Subtitle() { return _subtitle; }
	const struct Motif*   RandomMotif();
	const struct Motif*   GetMotif(int index);
};

MapInfo       MapFetch(int level);
bool          MapIsChapter(int map);

#endif // MAP_H