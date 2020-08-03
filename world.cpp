/*-----------------------------------------------------------------------------

  World.cpp

  This controls the loading, populating, and rendering of levels.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "camera.h"
#include "collision.h"
#include "entity.h"
#include "env.h"
#include "game.h"
#include "main.h"
#include "map.h"
#include "menu.h"
#include "page.h"
#include "particle.h"
#include "player.h"
#include "random.h"
#include "render.h"
#include "system.h"
#include "texture.h"
#include "trivia.h"
#include "visible.h"
#include "world.h"
#include "zone.h"

#define FADE_TIME           1500
#define MAX_TILT						2.0f
#define TILT_SPEED					0.03f

enum FadeState
{
	FADE_NONE,
	FADE_IN,
	FADE_OUT
};

static Texture*             tx_front;
static Texture*             tx_back1;
static Texture*             tx_back2;
static Texture*             tx_sky;
static GLcoord2             update_start;
static GLcoord2             update_end;
static GLrgba               sky_flash;

//When a vending machine is invoked, it sets this value
//When a menu is selected and an item is purchased, this value is used.
static GLvector2						item_dropoff;

//Used for fading the level title in / out when entering a new area.
static GLrgba               level_title_color;
static int                  level_title_start;
static int                  level_title_index;
static bool                 level_title_visible;
static float                level_title_fade;
static string               level_title_text;
static string               level_title_sub;

//Use for displaying the boss HP bar.
static BossInfo             boss_info;
static bool                 boss_active;
static bool                 boss_updated;
static bool                 boss_music_active;

static GLcoord2             player_page;
static GLbbox2              world_bounds;

static Map                  current_map;
static int                  current_map_index;
static Zone                 current_zone;
static int                  current_zone_index;

static bool									final_boss_dead;

//Used when fading in and out between levels.
static FadeState            fade_state;
static int                  fade_start;
static float                fade_value;
static int                  fade_destination;

//Used for switching vending machine music.
static int									vending_music_timeout;
static bool									vending_music;

//The sea of dust particles around the player.
static fxDust               dust;

//Camera tilt
static float								current_tilt;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void do_red_fade(float fade)
{
	GLcoord2    size;

	if (fade <= 0.0f)
		return;
	size = RenderViewportSize();
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	glDisable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f - fade, 1.0f - fade);
	glBegin(GL_QUADS);
	glVertex2d(0, 0);
	glVertex2d(size.x, 0);
	glVertex2d(size.x, size.y);
	glVertex2d(0, size.y);
	glEnd();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
}

static void draw_fade(float opacity)
{
	if (!fade_state)
		return;
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_TEXTURE_2D);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0, 0, 0, opacity);
	glBegin(GL_QUADS);
	glVertex3f(-100, -100, DEPTH_OVERLAY);
	glVertex3f(100, -100, DEPTH_OVERLAY);
	glVertex3f(100, 100, DEPTH_OVERLAY);
	glVertex3f(-100, 100, DEPTH_OVERLAY);
	glEnd();
	glEnable(GL_TEXTURE_2D);
}

//The player has changed zones. Load in the new one and prepare it for play.
static void do_zone(int zone)
{
	vector<ZoneExitDoor>    exits;
	ZoneExitDoor            ze;

	ze.sprite = SPRITE_ALERT;
	ze.zone_id = 0;
	exits.push_back(ze);

	//We need a list of between 1 and 3 doors for the end of this level. Each door
	//will lead to a different zone of this map. Once the player beats the boss, they
	//will be taken to the next map, even if there are still zones they haven't
	//completed.
	vector<ZoneInfo>      zone_list = *current_map.Zones();
	vector<ZoneExitDoor>  possible_doors;
	vector<ZoneExitDoor>  chosen_doors;
	ZoneExitDoor          door;
	int                   completed_zones;
	bool                  door_to_boss;
	bool									final_zone;

	completed_zones = 0;
	current_zone_index = zone;
	Player()->CheckpointSet(Checkpoint(current_map_index, current_zone_index, 0));
	//We begin by getting a list of all the zones that are still available.
	//First, add doors for every zone EXCEPT the last one. (Which would lead to the boss.)
	for (unsigned z = 0; z < zone_list.size() - 1; z++) {
		//Can't re-do completed zones.
		if (Player()->ZoneComplete(zone_list[z]._map_id, zone_list[z]._zone_id)) {
			completed_zones++;
			continue;
		}
		//A zone shouldn't have a door to itself!
		if (z == zone)
			continue;
		//The last zone is the "boss".
		door.sprite = zone_list[z]._door_sprite;
		door.zone_id = z;
		possible_doors.push_back(door);
	}

	//Randomly shuffle the list so the same zones do not get picked when duplicates are removed
	std::random_shuffle(possible_doors.begin(), possible_doors.end());

	//Remove zones with duplicate icons here
	for (int i = 0; i < possible_doors.size(); i++)
	{
		for (int j = i + 1; j < possible_doors.size(); j++)
		{
			if (possible_doors.at(j).sprite == possible_doors.at(i).sprite)
			{
				vector<ZoneExitDoor>::iterator iter = possible_doors.begin() + j;
				possible_doors.erase(iter);
				--j;
			}
		}
	}

	//Figure out if the door to the final zone should be available.
	door_to_boss = false;
	//If this zone is short, then go ahead and include the boss door.
	if (zone_list.size() < 4)
		door_to_boss = true;
	else if (completed_zones >= current_map.ZonesMin()) // make sure we the minimum number before the boss is available.
		door_to_boss = true;
	if (door_to_boss) {
		int z = zone_list.size() - 1;
		door.sprite = zone_list[z]._door_sprite;
		door.zone_id = z;
		possible_doors.push_back(door);
	}
	//We may choose to obscure a door icon 1 out of 3 times. But only if:
	//1. If there's more than one door available
	//2. We're not in the first zone of a level
	//NOTE: The actual probability is less than 33%, because the mystery door may not be chosen as one of the 3 doors.
	//The final probability is 25% for 4 distinct icons, and 12.5% for 5 distinct icons and so on
	if (RandomVal(3) == 0 && possible_doors.size() > 1 && current_zone_index > 0) {
		int	random_door = RandomVal(possible_doors.size());
		possible_doors[random_door].sprite = SPRITE_DOOR_MYSTERY;
	}
	//If the player has hit the limit, take away their choices and MAKE them face the boss.
	if (completed_zones > current_map.ZonesMax()) {
		int z = zone_list.size() - 1;
		door.sprite = zone_list[z]._door_sprite;
		door.zone_id = z;
		possible_doors.clear();
		possible_doors.push_back(door);
	}
	//Now that we have a list of doors, try to pick 3.
	while (!possible_doors.empty() && chosen_doors.size() < 3) {
		unsigned r = RandomVal(possible_doors.size());

		//Door [r], I choose you!
		chosen_doors.push_back(possible_doors[r]);
		possible_doors.erase(possible_doors.begin() + r);
	}
	//if we're on the very last zone, don't offer any doors back to possible missed zones.
	//Just offer one door to change levels.
	final_zone = zone == current_map.Zones()->size() - 1;
	if (final_zone) {
		chosen_doors.clear();
		door.sprite = SPRITE_DOOR_EXTRA;
		door.zone_id = ZONE_NEXT_LEVEL;
		chosen_doors.push_back(door);
	}

	const Motif*	mot = current_map.RandomMotif();

	mot = current_zone.Init(&current_map.Zones()->at(zone), mot, chosen_doors);
	if (!mot->_texture_fore.empty())
		tx_front = TextureFromName(mot->_texture_fore);
	else
		tx_front = TextureFromName(current_map.TextureName(TEXTURE_FRONT));
	if (!mot->_texture_mid.empty())
		tx_back1 = TextureFromName(mot->_texture_mid);
	else
		tx_back1 = TextureFromName(current_map.TextureName(TEXTURE_MIDDLE));
	if (!mot->_texture_back.empty())
		tx_back2 = TextureFromName(mot->_texture_back);
	else
		tx_back2 = TextureFromName(current_map.TextureName(TEXTURE_BACK));
	if (!mot->_texture_sky.empty())
		tx_sky = TextureFromName(mot->_texture_sky);
	else
		tx_sky = TextureFromName(current_map.TextureName(TEXTURE_SKY));

	EntityClear();
	current_zone.Activate(final_zone);
	AudioPlaySong(current_map.Music(zone));
	fade_start = GameTick();
	fade_state = FADE_IN;
	dust.Init();
	Player()->WorldCheckAch(current_map_index, current_zone_index);
	GameSave();
}

//Load the given level. This may be the start of the game, loading a game,
//triggered by console commands, or simply part of a player-activated level change.
static void do_level(int level)
{
	GLcoord2    grid;
	GLcoord2    prev;
	GLcoord2    neighbor;
	PageInfo    si;
	GLvector2   spawn_point;
	Robot       b;
	Map         m;

	current_map.Init(level);
	EntityClear();
	world_bounds.Clear();
	final_boss_dead = false;
	current_zone_index = 0;
	current_map_index = level;
	do_zone(0);
	Player()->CheckpointSet(Checkpoint(level, 0, 0));
	GameSave();
	PlayerSpawn(current_zone.Entry());
}

void do_level_change(int level_number)
{
	GameSave();
	level_title_index = level_number;
	level_title_start = GameTick();
	level_title_visible = true;
	level_title_color = GLrgba();//Start with black, then fade in.
}

void do_level_title()
{
	GLcoord2    view_size;
	GLcoord2    pos;
	int         elapsed;
	float       fade;
	string      title;
	string      subtitle;

	title = current_map.Title();
	subtitle = current_map.Subtitle();
	elapsed = GameTick() - level_title_start;
	if (elapsed > 10000)
		level_title_visible = false;
	if (elapsed < 3000)
		fade = (float)elapsed / 3000;
	else if (elapsed > 7000)
		fade = (float)(10000 - elapsed) / 3000;
	else
		fade = 1.0f;
	level_title_fade = fade;
	level_title_color = GLrgba(fade, fade, fade);
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

int WorldRoomFromPosition(GLvector2 pos)
{
	return current_zone.RoomFromPosition(pos);
}

void WorldVendingNear()
{
	vending_music_timeout = 10;
}

bool WorldTitle(string& title, string &subtitle, float fade)
{
	if (!level_title_visible)
		return false;
	title = current_map.Title();
	subtitle = current_map.Subtitle();
	fade = level_title_fade;
	return true;
}

bool WorldTitleVisible() { return level_title_visible; }

Zone* WorldZone() { return &current_zone; }

bool WorldZoneClear ()
{
	if (WorldBossGet () != NULL)
		return false;
	if (!current_zone.SpawnersEmpty ())
		return false;
	for (int i=0; i< EntityRobotCount () ; i++) {
		Robot*	r = EntityRobot (i);
		if (!r->Dead ())
			return false;
	}
	return true;
}

const Map* WorldMap() { return &current_map; }

void WorldSetMotif(int index)
{
	vector<ZoneExitDoor>    doors;
	ZoneExitDoor            z;

	z.sprite = SPRITE_DOOR_LOCKED;
	z.zone_id = 0;
	doors.push_back(z);
	current_zone.Init(&current_map.Zones()->at(current_zone_index), current_map.GetMotif(index), doors);
	EntityClear();
	current_zone.Activate(false);
	fade_start = GameTick();
	fade_state = FADE_IN;
	dust.Init();
}

///This is called to trigger the fade to black when exiting a zone.
///When the fade is complete, the game will transition to the destination zone.
void WorldZoneTransition(int destination)
{
	if (destination >= current_map.Zones()->size())
		destination = ZONE_NEXT_LEVEL;
	fade_start = GameTick();
	fade_state = FADE_OUT;
	fade_destination = destination;
}

GLcoord2 WorldPlayerLocation()
{
	return GLcoord2(current_map_index, current_zone_index);
}

///Used to give every zone a unique ID.
int WorldLocationId()
{
	return current_map_index * 100 + current_zone_index + GameShoppingId();
}

int WorldLevelIndex() { return current_map_index; }
int WorldZoneIndex() { return current_zone_index; }

void WorldBossSet(BossInfo bi)
{
	if (!boss_music_active) {
		boss_music_active = true;
		AudioPlaySong(Env().music_boss.c_str());
	}
	boss_info = bi;
	boss_updated = true;
}

const BossInfo* WorldBossGet()
{
	const BossInfo*   result;

	if (!boss_active)
		return NULL;
	result = &boss_info;
	return result;
}

bool WorldCellSolid(GLcoord2 world)
{
	return current_zone.CellSolid(world);
}

short WorldCellShape(GLvector2 point)
{
	return current_zone.CellShape(GLcoord2((int)point.x, (int)point.y));
}

bool WorldCellEmpty(GLvector2 point)
{
	return current_zone.CellShape(GLcoord2((int)point.x, (int)point.y)) == 0;
}

GLvector2 WorldLanding(Checkpoint checkpoint)
{
	return current_zone.Respawn();
}

GLrgba WorldLampColor() { return current_zone.Color(COLOR_LAMP); }

GLbbox2 WorldBounds() { return current_zone.Bounds(); }

//TODO: This should trigger a re-compile of the zones.
void WorldValidate()
{
	current_zone.Compile();
}

void WorldSkyFlash(float magnitude)
{
	sky_flash += GLrgba(magnitude, magnitude, 0);
}

void WorldUpdate()
{
	GLvector  camera;
	GLvector2 player;
	GLvector2 shove;

	if (!GameActive())
		return;
	vending_music_timeout--;
	if (vending_music && vending_music_timeout < 1) {
		vending_music = false;
		AudioPlaySong(current_map.Music(current_zone_index));
	}
	if (boss_music_active && !boss_active) {
		AudioPlaySong(current_map.Music(current_zone_index));
		boss_music_active = false;
	}

	if (!vending_music && vending_music_timeout >= 1) {
		vending_music = true;
		AudioPlaySong(Env().music_shop.c_str());
	}
	dust.Update();
	sky_flash *= 0.95f;
	boss_updated = false;
	EntityUpdate();
	boss_active = boss_updated;
	camera = CameraPosition();
	player = PlayerPosition();
	player_page.x = (int)player.x / PAGE_SIZE;
	player_page.y = (int)player.y / PAGE_SIZE;
	if (level_title_visible)
		do_level_title();
	//Do the fade in / out between zones.
	if (fade_state) {
		int   delta = GameTick() - fade_start;
		if (fade_state == FADE_IN) {
			fade_value = 1.0f - ((float)delta / (float)FADE_TIME);
			if (delta >= FADE_TIME)
				fade_state = FADE_NONE;
		}
		if (fade_state == FADE_OUT) {
			fade_value = ((float)delta / (float)FADE_TIME);
			if (delta >= FADE_TIME) {
				EntityClear();
				fade_state = FADE_NONE;
				if (final_boss_dead) {
					GameEnd();
					MenuOpen(MENU_WIN);
					Player()->WinCheckAch();
					Player()->SaveHighScore();
				}
				else if (fade_destination == ZONE_NEXT_LEVEL) {
					WorldMapSet(current_map_index + 1, 0);
				}
				else //Not changing levels, just proceed to next zone.
					do_zone(fade_destination);
			}
		}
	}
}

/*-----------------------------------------------------------------------------
This is where the gameworld is drawn. (Not including menus and HUD elements.
-----------------------------------------------------------------------------*/

void WorldRender()
{
	GLvector  eye;
	GLrgba    clear;

	//If the game isn't running, just clear the screen.
	if (!GameRunning()) {
		glClearColor(0, 0, 0, 1.0f);
		glStencilMask(0xff);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		return;
	}
	//Position the camera, clear the buffers, get ready to draw.
	eye = CameraPosition();
	GLrgba color_sky = current_zone.Color(COLOR_SKY);
	glClearColor(color_sky.red, color_sky.green, color_sky.blue, 1.0f);
	glStencilMask(0xff);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glStencilMask(0x0);
	glLoadIdentity();
	glScalef(1, -1, 1);
	float desired_tilt = PlayerMomentum().x * 20.0f;
	desired_tilt = clamp(desired_tilt, -MAX_TILT, MAX_TILT);
	current_tilt = Lerp(current_tilt, desired_tilt, TILT_SPEED);
	if (!EnvValueb(ENV_NOTILT))
		glRotatef(current_tilt, 0.0f, 0.0f, 1.0f);
	glTranslatef(-eye.x, -eye.y, -eye.z);

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Draw the glowing aura around the player,
	RenderQuad(GLvector2(eye.x, eye.y), SPRITE_GLOW, GLrgba(1, 1, 1), eye.z * 2 * RenderAspect(), 0, DEPTH_FX_GLOW, true);
	RenderQuads();

	//We render the avatar's line of sight vision to get the imprint on the stencil buffer.
	RenderStencilImprint(STENCIL_OCCLUSION);
	VisibleRender();
	RenderStencilMask(0, STENCIL_OCCLUSION);

	//Imprint the lamp cone on the stencil buffer.
	glEnable(GL_TEXTURE_2D);
	RenderStencilImprint(STENCIL_LAMP);
	RenderWrite(false);
	glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
	VisibleRenderCone(0.15f, DEPTH_FX_GLOW);
	RenderStencilMask(0, STENCIL_OCCLUSION);
	RenderWrite(true);

	//Draw the scrolling background texture.
	glBindTexture(GL_TEXTURE_2D, tx_sky->Id());
	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	current_zone.RenderSky();

	//Draw the outer walls in the distance.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	current_zone.Render(PAGE_LAYER_OUTER, tx_back2->Id());
	VisibleRenderCone(0.15f, DEPTH_UNIT_GLOW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	current_zone.Render(PAGE_LAYER_INNER, tx_back1->Id());
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(false);
	current_zone.Render(PAGE_LAYER_GLOW, tx_front->Id());
	glDepthMask(true);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
	glDisable(GL_STENCIL_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (EnvValueb(ENV_SHADOWS))
		glEnable(GL_STENCIL_TEST);
	glEnable(GL_DEPTH_TEST);

	//Draw dust particles.
	glEnable(GL_STENCIL_TEST);
	glDepthMask(false);
	//Draw them ONLY in the players light cone, according to lamp color.
	RenderStencilMask(STENCIL_LAMP, STENCIL_LAMP | STENCIL_OCCLUSION);
	dust.Render(current_zone.Color(COLOR_LAMP));
	RenderQuads();///Flush the current queue before we change the render settings.
	//Now draw them everywhere we can see, according to background color.
	RenderStencilMask(0, STENCIL_OCCLUSION);
	dust.Render(current_zone.Color(COLOR_SKY));
	RenderQuads();///Flush the current queue before we change the render settings.

	glDepthMask(true);

	if (current_zone.Blind())
		RenderStencilMask(STENCIL_LAMP, STENCIL_LAMP | STENCIL_OCCLUSION);
	EntityRenderRobots(false);
	RenderQuads();///Flush the current queue before we change the render settings.
	glDisable(GL_STENCIL_TEST);//The player can ALWAYS see themselves!
	PlayerRender();

	glDepthMask(true);
	VisibleRenderCone(0.15f, DEPTH_FX);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Doors are visible through walls, since they are kinda walls themselves.
	glDisable(GL_STENCIL_TEST);
	glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
	glDepthMask(true);
	EntityDeviceRender(true);
	glDepthMask(true);
	//Now render the various entities.
	RenderStencilMask(0, STENCIL_OCCLUSION);
	glEnable(GL_STENCIL_TEST);
	EntityDeviceRender(false);
	EntityRenderFx();
	RenderQuads();///Flush the current queue before we change the render settings.
	if (Player()->Ability(ABILITY_SCANNER)) {
		if (current_zone.Blind())
			glStencilFunc(GL_NOTEQUAL, STENCIL_LAMP, STENCIL_OCCLUSION | STENCIL_LAMP);
		else
			glStencilFunc(GL_EQUAL, STENCIL_OCCLUSION, STENCIL_OCCLUSION);
		//glStencilFunc (GL_NOTEQUAL, STENCIL_LAMP, STENCIL_LAMP);
		EntityRenderRobots(true);
		RenderQuads();///Flush the current queue before we change the render settings.
	}
	VisibleInvert(false);
	ParticleRender();
	current_zone.Render(PAGE_LAYER_DEBUG, SpriteMapTexture());
	glDepthMask(false);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	current_zone.Render(PAGE_LAYER_MAIN, tx_front->Id());
	if (!EnvValueb(ENV_BBOX))
		current_zone.Render(PAGE_LAYER_DEBUG, tx_front->Id());
	glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());

	draw_fade(fade_value);
	TriviaRender();
	CollisionRender();
	if (EnvValueb(ENV_LOS))
		VisibleRenderLOS();
}

void WorldInit()
{
}

void WorldFinalBossKill()
{
	final_boss_dead = true;
	AudioPlaySong(Env().music_win.c_str());
}

bool WorldFinalBossKilled()
{
	return final_boss_dead;
}

void WorldRender2D()
{
	if (level_title_visible)
		do_level_title();
	do_red_fade(PlayerDeathFade());
}

void WorldMapSet(int index, int zone)
{
	Checkpoint    player_checkpoint;

	//Set the current map, but make sure to not set it to an invalid number
	current_map_index = index % EnvMapCount();
	do_level(current_map_index);

	//Set the current zone
	current_zone_index = zone % current_map.Zones()->size();
	do_zone(current_zone_index);

	player_checkpoint = Player()->CheckpointGet();

	//If we're suddenly jumping to a level that's NOT where the player left off,
	//then the player is level-skipping. Maybe cheats or chapter select. Whatever.
	//Just start them at the beginning of this new level.
	if (player_checkpoint.map != index)
		player_checkpoint = Checkpoint(index, 0, 0);
	do_level_change(index);
	boss_music_active = false;

	Player()->WorldCheckAch(current_map_index, current_zone_index);
}

const class Page* WorldPage(GLcoord2 pos)
{
	return current_zone.PageGet(pos);
}

GLvector2 WorldItemDropoff()
{
	return item_dropoff;
}

void WorldItemDropoffSet(GLvector2 val)
{
	item_dropoff = val;
}