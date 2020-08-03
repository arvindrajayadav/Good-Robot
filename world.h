#ifndef WORLD_H
#define WORLD_H

struct BossInfo
{
	const char*     name;
	GLvector2       position;
	int             hp;
	int             hp_max;
};

class Zone*       WorldZone();
const class Map*  WorldMap ();

const BossInfo*   WorldBossGet();
void              WorldBossSet(BossInfo bi);
GLbbox2           WorldBounds();

bool              WorldCellEmpty(GLvector2 point);
bool              WorldCellSolid(GLcoord2 world);
short             WorldCellShape(GLvector2 point);
void							WorldFinalBossKill ();
bool							WorldFinalBossKilled ();
GLvector2         WorldLanding(struct Checkpoint checkpoint);
GLrgba            WorldLampColor();

bool              WorldIsCalm();

GLvector2					WorldItemDropoff ();
void							WorldItemDropoffSet (GLvector2);

void              WorldInit();
GLcoord2          WorldPlayerLocation();
int               WorldLocationId();

int               WorldLevelIndex();
int               WorldZoneIndex();

void							WorldVendingNear ();

bool              WorldTitle(string& title, string &subtitle, float fade);
bool              WorldTitleVisible();
void              WorldRender();
void              WorldRender2D();
int               WorldRoomFromPosition(GLvector2 pos);
void              WorldSkyFlash(float magnitude);
void              WorldUpdate();
void              WorldValidate();
void              WorldMapSet(int level, int zone);
void              WorldZoneTransition(int zone);
bool							WorldZoneClear ();


void              WorldSetMotif(int index);
const class Page* WorldPage(GLcoord2 pos);
#endif // WORLD_H
