/*-----------------------------------------------------------------------------

  Sprite.cpp

  Good Robot
  (c) 2013 Shamus Young

  Used for keeping track of UV positions of individual sprites in a sprite bank.

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "ini.h"
#include "resource.h"
#include "texture.h"

#define SPRITE_FILE     "sprite.ini"

#define SPRITE_GRID     32
#define ONE_BLOCK       (1.0f / SPRITE_GRID)
#define HALF_PIXEL      (ONE_BLOCK / 128)
#define SPRITE_SHEET    "sprites.png"
#define SPRITE_ENTIRES  (sizeof (sprite_list) / sizeof (SpriteNameValuePair))

struct SpriteNameValuePair
{
	int       index;
	char*     entry;
} sprite_list[] =
{
	{ SPRITE_ROBOT0, "Robot0" },
	{ SPRITE_ROBOT1, "Robot1" },
	{ SPRITE_ROBOT2, "Robot2" },
	{ SPRITE_ROBOT3, "Robot3" },
	{ SPRITE_ROBOT4, "Robot4" },
	{ SPRITE_ROBOT5, "Robot5" },
	{ SPRITE_ROBOT6, "Robot6" },
	{ SPRITE_ROBOT7, "Robot7" },
	{ SPRITE_ROBOT8, "Robot8" },
	{ SPRITE_ROBOT9, "Robot9" },
	{ SPRITE_ROBOT10, "Robot10" },
	{ SPRITE_ROBOT11, "Robot11" },
	{ SPRITE_ROBOT12, "Robot12" },
	{ SPRITE_ROBOT13, "Robot13" },
	{ SPRITE_ROBOT14, "Robot14" },
	{ SPRITE_ROBOT15, "Robot15" },
	{ SPRITE_ROBOT16, "Robot16" },
	{ SPRITE_ROBOT17, "Robot17" },
	{ SPRITE_ROBOT18, "Robot18" },
	{ SPRITE_ROBOT19, "Robot19" },
	{ SPRITE_ROBOT20, "Robot20" },
	{ SPRITE_ROBOT21, "Robot21" },
	{ SPRITE_ROBOT22, "Robot22" },
	{ SPRITE_ROBOT23, "Robot23" },
	{ SPRITE_ROBOT24, "Robot24" },
	{ SPRITE_ROBOT25, "Robot25" },
	{ SPRITE_ROBOT26, "Robot26" },
	{ SPRITE_ROBOT27, "Robot27" },
	{ SPRITE_ROBOT28, "Robot28" },
	{ SPRITE_ROBOT29, "Robot29" },
	{ SPRITE_ROBOT30, "Robot30" },
	{ SPRITE_ROBOT31, "Robot31" },
	//Boss parts
	{ SPRITE_BOSS0, "Boss0" },
	{ SPRITE_BOSS1, "Boss1" },
	{ SPRITE_BOSS2, "Boss2" },
	{ SPRITE_BOSS3, "Boss3" },
	{ SPRITE_BOSS4, "Boss4" },
	{ SPRITE_BOSS5, "Boss5" },
	{ SPRITE_BOSS6, "Boss6" },
	{ SPRITE_BOSS7, "Boss7" },
	{ SPRITE_BOSS8, "Boss8" },
	{ SPRITE_BOSS9, "Boss9" },
	{ SPRITE_BOSS10, "Boss10" },
	{ SPRITE_BOSS11, "Boss11" },
	{ SPRITE_BOSS12, "Boss12" },
	{ SPRITE_BOSS13, "Boss13" },
	{ SPRITE_BOSS14, "Boss14" },
	{ SPRITE_BOSS15, "Boss15" },

	{ SPRITE_EYE0, "Eye0" },
	{ SPRITE_EYE1, "Eye1" },
	{ SPRITE_EYE2, "Eye2" },
	{ SPRITE_EYE3, "Eye3" },
	{ SPRITE_EYE4, "Eye4" },
	{ SPRITE_EYE5, "Eye5" },
	{ SPRITE_EYE6, "Eye6" },
	{ SPRITE_EYE7, "Eye7" },
	{ SPRITE_EYE8, "Eye8" },
	{ SPRITE_EYE9, "Eye9" },
	{ SPRITE_IRIS0, "Iris0" },
	{ SPRITE_IRIS1, "Iris1" },
	{ SPRITE_IRIS2, "Iris2" },
	{ SPRITE_IRIS3, "Iris3" },
	{ SPRITE_IRIS4, "Iris4" },
	{ SPRITE_IRIS5, "Iris5" },
	{ SPRITE_IRIS6, "Iris6" },
	{ SPRITE_IRIS7, "Iris7" },
	{ SPRITE_IRIS8, "Iris8" },
	{ SPRITE_IRIS9, "Iris9" },

	//Body parts
	{ SPRITE_HEAD1, "Head1" },
	{ SPRITE_HEAD2, "Head2" },
	{ SPRITE_HEAD3, "Head3" },
	{ SPRITE_HEAD4, "Head4" },
	{ SPRITE_HEAD5, "Head5" },
	{ SPRITE_HEAD6, "Head6" },
	{ SPRITE_TORSO1, "Torso1" },
	{ SPRITE_TORSO2, "Torso2" },
	{ SPRITE_TORSO3, "Torso3" },
	{ SPRITE_TORSO4, "Torso4" },
	{ SPRITE_TORSO5, "Torso5" },
	{ SPRITE_TORSO6, "Torso6" },
	{ SPRITE_CORE, "Core" },
	{ SPRITE_LASER1, "Laser1" },
	{ SPRITE_LASER2, "Laser2" },
	{ SPRITE_LASER3, "Laser3" },
	{ SPRITE_LASER4, "Laser4" },
	{ SPRITE_LASER5, "Laser5" },
	{ SPRITE_LASER6, "Laser6" },
	{ SPRITE_KNEE, "Knee" },
	{ SPRITE_LAUNCHER1, "Launcher1" },
	{ SPRITE_LAUNCHER2, "Launcher2" },
	{ SPRITE_LAUNCHER3, "Launcher3" },
	{ SPRITE_LAUNCHER4, "Launcher4" },
	{ SPRITE_LAUNCHER5, "Launcher5" },
	{ SPRITE_LAUNCHER6, "Launcher6" },
	{ SPRITE_LAUNCHER7, "Launcher7" },
	{ SPRITE_LAUNCHER8, "Launcher8" },
	//FX
	{ SPRITE_SHOCKWAVE, "Shockwave" },
	{ SPRITE_SPARK, "Spark" },
	{ SPRITE_FADE, "Fade" },
	{ SPRITE_GLOW, "Glow" },
	{ SPRITE_SHIELD, "Shield" },
	{ SPRITE_ORB, "Orb" },
	{ SPRITE_TAIL, "Tail" },
	{ SPRITE_DEBRIS1, "Debris1" },
	{ SPRITE_DEBRIS2, "Debris2" },
	{ SPRITE_RUBBLE1, "Rubble1" },
	{ SPRITE_RUBBLE2, "Rubble2" },
	{ SPRITE_SMOKE, "Smoke" },
	{ SPRITE_FOG, "Fog" },
	{ SPRITE_FLASH, "Flash" },
	{ SPRITE_LANDING, "Landing" },
	{ SPRITE_CREDITS, "Credits" },
	//Projectiles
	{ SPRITE_PROJECTILE1, "Projectile1" },
	{ SPRITE_PROJECTILE2, "Projectile2" },
	{ SPRITE_PROJECTILE3, "Projectile3" },
	{ SPRITE_PROJECTILE4, "Projectile4" },
	{ SPRITE_PROJECTILE5, "Projectile5" },
	{ SPRITE_PROJECTILE6, "Projectile6" },
	{ SPRITE_PROJECTILE7, "Projectile7" },
	{ SPRITE_PROJECTILE8, "Projectile8" },

	//HUD
	{ SPRITE_COIN, "Coin" },
	{ SPRITE_TARGET, "Target" },
	{ SPRITE_BAR, "Bar" },
	{ SPRITE_MISSILE, "Missile" },
	{ SPRITE_WARNING, "Warning" },
	{ SPRITE_ARROW, "Arrow" },
	{ SPRITE_XP_BAR, "Bar" },
	{ SPRITE_BEAM, "Beam" },
	{ SPRITE_ALERT, "Alert" },
	{ SPRITE_STORY, "Story" },
	//Projectile
	{ SPRITE_DUMBFIRE, "Dumbfire" },
	{ SPRITE_HOMING, "Homing" },
	{ SPRITE_CLUSTER, "Cluster" },
	{ SPRITE_NUKE, "Nuke" },
	{ SPRITE_NOVA, "Nova" },
	{ SPRITE_BULLET, "Bullet" },
	{ SPRITE_GEAR, "Gear" },
	{ SPRITE_MOTE, "Mote" },
	//Doors
	{ SPRITE_DOOR, "Door" },
	{ SPRITE_DOOR_MYSTERY, "DoorMystery" },
	{ SPRITE_DOOR_BOSS, "DoorBoss" },
	{ SPRITE_DOOR_LOCKED, "DoorLocked" },
	{ SPRITE_DOOR_ROBOTS, "DoorRobots" },
	{ SPRITE_DOOR_SHOP, "DoorShop" },
	{ SPRITE_DOOR_EXTRA, "DoorExtra" },

	//Gun pickup icons
	{ SPRITE_GUN1, "Gun1" },
	{ SPRITE_GUN2, "Gun2" },
	{ SPRITE_GUN3, "Gun3" },
	{ SPRITE_GUN4, "Gun4" },
	{ SPRITE_GUN5, "Gun5" },
	{ SPRITE_GUN6, "Gun6" },
	{ SPRITE_GUN7, "Gun7" },
	{ SPRITE_GUN8, "Gun8" },
	{ SPRITE_GUN9, "Gun9" },
	{ SPRITE_GUN10, "Gun10" },
	{ SPRITE_GUN11, "Gun11" },
	{ SPRITE_GUN12, "Gun12" },
	{ SPRITE_GUN13, "Gun13" },
	{ SPRITE_GUN14, "Gun14" },
	{ SPRITE_GUN15, "Gun15" },
	{ SPRITE_GUN16, "Gun16" },
	{ SPRITE_GUN17, "Gun17" },
	{ SPRITE_GUN18, "Gun18" },
	{ SPRITE_GUN19, "Gun19" },
	{ SPRITE_GUN20, "Gun20" },
	{ SPRITE_GUN21, "Gun21" },
	{ SPRITE_GUN22, "Gun22" },
	{ SPRITE_GUN23, "Gun23" },
	{ SPRITE_GUN24, "Gun24" },
	{ SPRITE_GUN25, "Gun25" },
	{ SPRITE_GUN26, "Gun26" },
	{ SPRITE_GUN27, "Gun27" },
	{ SPRITE_GUN28, "Gun28" },
	{ SPRITE_GUN29, "Gun29" },
	{ SPRITE_GUN30, "Gun30" },
	{ SPRITE_GUN31, "Gun31" },
	{ SPRITE_GUN32, "Gun32" },

	//Hands for the avatar.
	{ SPRITE_HAND1, "Hand1" },
	{ SPRITE_HAND2, "Hand2" },
	{ SPRITE_HAND3, "Hand3" },
	{ SPRITE_HAND4, "Hand4" },
	{ SPRITE_HAND5, "Hand5" },
	{ SPRITE_HAND6, "Hand6" },
	{ SPRITE_HAND7, "Hand7" },
	{ SPRITE_HAND8, "Hand8" },
	{ SPRITE_HAND9, "Hand9" },
	{ SPRITE_HAND10, "Hand10" },
	{ SPRITE_HAND11, "Hand11" },
	{ SPRITE_HAND12, "Hand12" },
	{ SPRITE_HAND13, "Hand13" },
	{ SPRITE_HAND14, "Hand14" },
	{ SPRITE_HAND15, "Hand15" },
	{ SPRITE_HAND16, "Hand16" },
	{ SPRITE_HAND17, "Hand17" },
	{ SPRITE_HAND18, "Hand18" },
	{ SPRITE_HAND19, "Hand19" },
	{ SPRITE_HAND20, "Hand20" },
	{ SPRITE_HAND21, "Hand21" },
	{ SPRITE_HAND22, "Hand22" },
	{ SPRITE_HAND23, "Hand23" },
	{ SPRITE_HAND24, "Hand24" },
	{ SPRITE_HAND25, "Hand25" },
	{ SPRITE_HAND26, "Hand26" },
	{ SPRITE_HAND27, "Hand27" },
	{ SPRITE_HAND28, "Hand28" },
	{ SPRITE_HAND29, "Hand29" },
	{ SPRITE_HAND30, "Hand30" },
	{ SPRITE_HAND31, "Hand31" },
	{ SPRITE_HAND32, "Hand32" },

	//Large machine frames
	{ SPRITE_MACHINE1, "Machine1" },
	{ SPRITE_MACHINE2, "Machine2" },
	{ SPRITE_MACHINE3, "Machine3" },
	{ SPRITE_MACHINE4, "Machine4" },
	{ SPRITE_MACHINE5, "Machine5" },
	{ SPRITE_MACHINE6, "Machine6" },
	{ SPRITE_MACHINE7, "Machine7" },
	{ SPRITE_MACHINE8, "Machine8" },
	{ SPRITE_MACHINE9, "Machine9" },
	{ SPRITE_MACHINE10, "Machine10" },
	{ SPRITE_MACHINE11, "Machine11" },
	{ SPRITE_MACHINE12, "Machine12" },
	{ SPRITE_MACHINE13, "Machine13" },
	{ SPRITE_MACHINE14, "Machine14" },
	{ SPRITE_MACHINE15, "Machine15" },
	{ SPRITE_MACHINE16, "Machine16" },
	{ SPRITE_MACHINE17, "Machine17" },
	{ SPRITE_MACHINE18, "Machine18" },
	{ SPRITE_MACHINE19, "Machine19" },
	{ SPRITE_MACHINE20, "Machine20" },
	{ SPRITE_MACHINE21, "Machine21" },
	{ SPRITE_MACHINE22, "Machine22" },
	{ SPRITE_MACHINE23, "Machine23" },
	{ SPRITE_MACHINE24, "Machine24" },
	{ SPRITE_MACHINE25, "Machine25" },
	{ SPRITE_MACHINE26, "Machine26" },
	{ SPRITE_MACHINE27, "Machine27" },
	{ SPRITE_MACHINE28, "Machine28" },
	{ SPRITE_MACHINE29, "Machine29" },
	{ SPRITE_MACHINE30, "Machine30" },
	{ SPRITE_MACHINE31, "Machine31" },
	{ SPRITE_MACHINE32, "Machine32" },

	//Small machine parts
	{ SPRITE_PART1, "Part1" },
	{ SPRITE_PART2, "Part2" },
	{ SPRITE_PART3, "Part3" },
	{ SPRITE_PART4, "Part4" },
	{ SPRITE_PART5, "Part5" },
	{ SPRITE_PART6, "Part6" },
	{ SPRITE_PART7, "Part7" },
	{ SPRITE_PART8, "Part8" },

	//BIG sprites
	{ SPRITE_SCREEN, "Screen" },
	{ SPRITE_SCANLINES, "Scanlines" },
	{ SPRITE_SNOW, "Snow" },

	{ SPRITE_STORY1, "Story1" },

	{ SPRITE_INVALID, "Invalid" },
};

struct Sprite
{
	string        _name;
	AtlasRef      _atlas;
	GLuvFrame     _frame;
	GLvector2     _atlas_location;

	void          Create(string name, float col, float row, float scale);
};

void Sprite::Create(string name, float col, float row, float scale)
{
	GLvector2     sprite_size = GLvector2(1.0f / SPRITE_GRID, 1.0f / SPRITE_GRID);
	GLvector2     half_pixel = GLvector2(HALF_PIXEL, HALF_PIXEL);
	GLvector2     origin;

	_name = name;
	_atlas.col = col;
	_atlas.row = row;
	_atlas.scale = max(scale, 1.0f);
	_atlas_location.x = col + (row * (SPRITE_GRID / (int)_atlas.scale));
	_atlas_location.y = _atlas.scale;
	origin = (sprite_size * _atlas.scale) * GLvector2((float)col, (float)row);
	_frame.Set(origin + half_pixel, origin + (sprite_size*_atlas.scale) - half_pixel);
}

static vector<Sprite> sprites;
static GLuvFrame      sprite[SPRITE_GRID][SPRITE_GRID];
static GLvector2      sprite_size;
static GLquad         spinner[360];
static GLvector2      sincosvec[360];
static Texture*       tx;
static GLcoord2       sheet_size;
static bool*          sheet_alpha;
static bool           init_done;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

int SpriteMapTexture()
{
	return tx->Id();
}

//Convert the given UV value to pixel position, and return if that pixel in
//our atlas texture is opaque.  This is used for collision checking.
bool SpriteMapAlpha(GLvector2 uv)
{
	GLcoord2    pixel;

	pixel.x = (int)(uv.x * (float)sheet_size.x);
	pixel.y = (int)(uv.y * (float)sheet_size.y);
	return sheet_alpha[pixel.y*sheet_size.x + pixel.x];
}

void SpriteMapInit()
{
	int             group, index;
	GLvector2       origin;
	GLvector2       half_pixel;
	GLvector2       corners[4];
	iniFile         ini;
	const unsigned char* buffer;
	GLvector        packed;

	init_done = true;
	//To avoid filtering making our sprites "bleed" into each other at the edges,
	//we pull them away from the broundaries by one pixel.
	//If we change the size of the sprite sheet, be need to adjust this as well.
	sprite_size = GLvector2(1.0f / SPRITE_GRID, 1.0f / SPRITE_GRID);
	half_pixel = GLvector2(HALF_PIXEL, HALF_PIXEL);
	//Our sprite list begins with the hard-coded enumerated list at the top of this file.
	//These are entries which MUST exist because they're referenced in the source code.
	//First, we fill in this based list with dummy values:
	ini.Open(ResourceLocation(SPRITE_FILE, RESOURCE_DATA));
	for (int i = 0; i < SPRITE_COUNT; i++) {
		Sprite    s;

		s.Create("err", 9, 5, 1);
		sprites.push_back(s);
	}
	//Now we pull in the values from our sprite file. If any sprites are overlooked
	//(perhaps an artist forgot or deleted them) then they'll remain at the dummy values,
	//which ought to make them stick out.
	for (int i = 0; i < SPRITE_ENTIRES; i++) {
		string      val;

		val = ini.StringGet("Map", sprite_list[i].entry);
		packed = GLvector();
		if (val.empty())
			continue;
		packed = ini.VectorGet("Map", sprite_list[i].entry);
		if (packed.IsZero()) //Hardcoded location of "ERR" tile on sprite sheet.
			packed = GLvector(9, 5, 0);
		sprites[sprite_list[i].index].Create(sprite_list[i].entry, packed.x, packed.y, packed.z);
	}
	//All the hardcoded, mandatory values exist. Now we go over the file and pull in
	//non-mandatory sprites added by artists.
	int  keys = ini.SectionKeys("Map");
	for (int k = 0; k < keys; k++) {
		string        name;
		SpriteEntry   e;
		Sprite        s;

		name = ini.SectionKey("Map", k);
		e = SpriteEntryLookup(name);
		//If this doesn't exist in the list yet, then it's new. Add it.
		if (e == SPRITE_INVALID) {
			packed = ini.VectorGet("Map", name);
			if (packed.IsZero()) //Hardcoded location of "ERR" tile on sprite sheet.
				packed = GLvector(9, 5, 0);
			s.Create(name, packed.x, packed.y, packed.z);
			sprites.push_back(s);
		}
	}
	//This is an alternate way of indexing the sprite sheet, used by super-old bits of code.
	for (group = 0; group < SPRITE_GRID; group++) {
		for (index = 0; index < SPRITE_GRID; index++) {
			if (group == SPRITE_SKY || group == SPRITE_BOSS) {
				origin = sprite_size * GLvector2((float)index, (float)group) * GLvector2(2, 1);
				sprite[group][index].Set(origin + half_pixel, origin - half_pixel + sprite_size * 2);
			}
			else { //normal-sized sprites
				origin = sprite_size * GLvector2((float)index, (float)group);
				sprite[group][index].Set(origin + half_pixel, origin + sprite_size - half_pixel);
			}
		}
	}
	Console("SpriteInit: Loaded %d sprites from %s", sprites.size(), SPRITE_SHEET);
	//Now we load in the texture...
	tx = TextureFromName(SPRITE_SHEET);
	buffer = (unsigned char*)tx->Data();
	//Build an array of bool values for the sprite sheet based on pixel alpha.
	//This is used for per-pixel hit detection.
	sheet_size = tx->Size();
	sheet_alpha = new bool[sheet_size.x * sheet_size.y];
	for (int x = 0; x < sheet_size.x; x++) {
		for (int y = 0; y < sheet_size.y; y++) {
			int     index;
			unsigned char r, g, b, a;

			index = (y*sheet_size.x + x);
			r = buffer[index * 4 + 0];
			g = buffer[index * 4 + 1];
			b = buffer[index * 4 + 2];
			a = buffer[index * 4 + 3];
			sheet_alpha[index] = false;
			if (a > 0)
				sheet_alpha[index] = true;
		}
	}
	//We build a collection of 360 rectangles, all rotated. This is used in rare cases
	//for rendering things that don't work with our sprite shader. (The player's light
	//cone flashlight being the biggest example.)
	corners[0] = GLvector2(0.5f, -0.5f);
	corners[1] = GLvector2(-0.5f, -0.5f);
	corners[2] = GLvector2(-0.5f, 0.5f);
	corners[3] = GLvector2(0.5f, 0.5f);
	for (int a = 0; a < 360; a++) {
		float         angle;

		angle = (float)a * DEGREES_TO_RADIANS;
		sincosvec[a].x = cosf(angle);
		sincosvec[a].y = sinf(angle);
		for (int i = 0; i < 4; i++) {
			spinner[a].corner[i].x = corners[i].x * sincosvec[a].x + corners[i].y * sincosvec[a].y;
			spinner[a].corner[i].y = corners[i].x * sincosvec[a].y - corners[i].y * sincosvec[a].x;
		}
	}
}

GLquad SpriteMapQuad(int angle)
{
	return spinner[AngleLimit(angle)];
}

GLvector2 SpriteMapVectorRotate(GLvector2 v, int angle)
{
	GLvector2 original;

	original = v;
	angle = AngleLimit(angle);
	v.x = original.x * sincosvec[angle].x + original.y * sincosvec[angle].y;
	v.y = original.x * sincosvec[angle].y - original.y * sincosvec[angle].x;
	return v;
}

GLvector2 SpriteMapVectorFromAngle(int angle)
{
	angle = AngleLimit(angle);
	return sincosvec[angle];
}

GLuvFrame*  SpriteMapLookup(int group, int index)
{
	return &sprite[group][index];
}

const AtlasRef* SpriteAtlasRef(SpriteEntry i)
{
	return &sprites[i]._atlas;
}

GLuvFrame*  SpriteMapLookup(SpriteEntry s)
{
	return &sprites[s]._frame;
}

SpriteEntry SpriteEntryLookup(string name)
{
	//If init isn't done, then the list is empty. Consult the hardcoded list of
	//Mandatory values and see if we can find our value there.
	if (!init_done) {
		for (int i = 0; i < SPRITE_ENTIRES; i++) {
			if (!stricmp(name.c_str(), sprite_list[i].entry))
				return (SpriteEntry)sprite_list[i].index;
		}
		return SPRITE_INVALID;
	}
	for (int i = 0; i < sprites.size(); i++) {
		if (!stricmp(name.c_str(), sprites[i]._name.c_str()))
			return (SpriteEntry)i;
	}
	return SPRITE_INVALID;
}