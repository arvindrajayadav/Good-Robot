/*-----------------------------------------------------------------------------

  Entity.cpp

  This tracks the active elements of the gameworld: pickups, projectiles,
  robots, etc.

  Good Robot
  (c) 2014 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "entity.h"
#include "env.h"
#include "fx.h"
#include "player.h"
#include "projectile.h"
#include "render.h"
#include "robot.h"
#include "system.h"
#include "visible.h"
#include "world.h"

#define MAX_SHOVES          30

static vector<Robot>        bot;
static vector<Robot>        bot_queue;
static fxProjectile         projectiles[MAX_PROJECTILES];
static int                  projectile_top;
static int                  active_robots;
static int                  dead_robots;
static vector<fx*>          fx_list;
static vector<fxDevice*>    device_list;
static bool                 is_calm;     //True if we're on a peaceful screen and not in combat.
static bool                 in_update;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void insert_coin(GLvector2 position, int value)
{
	fxPowerup* c;

	c = new fxPowerup;
	c->Init(position, POWERUP_COIN, value);
	EntityFxAdd(c);
}

static void do_shoving ()
{
	static int		current_shover;
	static int		current_shovee;
	unsigned			max_shoves;
	float					allowed_distance;
	float					distance;
	GLvector2			delta;
	int						shoves;
	int						bots_checked;
	unsigned			b1, b2;
	bool					new_shover;

	if (bot.empty ())
		return;
	shoves = 0;
	bots_checked = 0;
	//Have the bots shove each other to keep them from stacking up into a deathball.
	//All bots...
	max_shoves = min ((int)bot.size (), MAX_SHOVES);
	while (shoves < max_shoves) {
		shoves++;
		current_shovee++;
		b1 = current_shover % bot.size ();
		b2 = current_shovee % bot.size ();
		new_shover = false;
		if (bot[b1].Dead ())
			new_shover = true;
		if (b1 == b2) 
			new_shover = true;
		if (new_shover) {
				current_shover++;
				current_shovee = current_shover;
				continue;
		}
		if (bot[b2].Dead ())
				continue;
		allowed_distance = bot[b1].Size () + bot[b2].Size ();
		delta = bot[b1].Position () - bot[b2].Position ();
		if (fabs (delta.x) > allowed_distance || fabs (delta.y) > allowed_distance)
			continue;
		distance = delta.Length ();
		if (distance < allowed_distance) {
			float strength = 1.0f - distance / allowed_distance;
			//delta.Normalize ();
			delta *= strength;
			bot[b2].Shove (delta * -1);
			bot[b1].Shove (delta);
		}
	}
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

//TODO: Whhhaaat is this doing here?
bool WorldIsCalm()
{
	return is_calm;
}

void EntityDeviceAdd (fxDevice* d)
{
  device_list.push_back (d);
}

int EntityDeviceCount () { return device_list.size ();  }


void EntityDeviceRender (bool occluded)
{
  if (occluded) {
    for (unsigned f = 0; f < device_list.size (); f++)
      device_list[f]->RenderOccluded ();
  } else {
    for (unsigned f = 0; f < device_list.size (); f++)
      device_list[f]->Render ();
  }
  RenderQuads ();
}


fxDevice* EntityDeviceFromId (int index)
{
  return device_list[index];
}

void EntityFxAdd(fx* item)
{
	fx_list.push_back(item);
}

//This adds a SINGLE projectile. Should be used by projectiles wanting to spawn children.
void EntityProjectileAdd (fxProjectile p)
{
  projectiles[projectile_top] = p;
  projectile_top = (projectile_top + 1) % MAX_PROJECTILES;
}

//This will fire a projectile according to the given properties. It will
//generate sounds and handle grouping. This should be used by the player and robots.
void EntityProjectileFire (fxOwner owner, const Projectile* p_info, GLvector2 origin, GLvector2 vector, int damage_level)
{
  int             count;
  fxProjectile    p;

  count = clamp (p_info->_volley, 1, 100);
  for (int i = 0; i < count; i++) {
    p.Init (owner, p_info, origin, vector, damage_level);
    p.BouncesSet (p_info->_bounces);
    EntityProjectileAdd (p);
  }
  AudioPlay (p_info->_sound, origin);
}

fxProjectile* EntityProjectileList () 
{
  return projectiles;
}

void EntityRobotAdd(Robot b)
{ 
	//If we're in the middle of an update, we might be walking the list of bots.
	//If so, queue the new bot and it'll be added next update.
	if (in_update)
		bot_queue.push_back(b);
	else //Not in update mode. Safe to mess with the list.
		bot.push_back(b);
}

int EntityRobotsActive () { return active_robots;  }
int EntityRobotsDead () { return dead_robots; }
int EntityRobotCount() {	return bot.size(); }

Robot* EntityRobot(int index)
{
	if (index >= (int)bot.size())
		return NULL;
	return &bot[index];
}

fx* WorldFx(unsigned index)
{
	if (index >= (int)fx_list.size())
		return NULL;
	return fx_list[index];
}

void EntityXpAdd(GLvector2 position, int change)
{
	while (change >= 1000) {
		insert_coin(position, 1000);
		change -= 1000;
	}
	while (change >= 100) {
		insert_coin(position, 100);
		change -= 100;
	}
	while (change >= 10) {
		insert_coin(position, 10);
		change -= 10;
	}
	while (change >= 1) {
		insert_coin(position, 1);
		change -= 1;
	}
}

void EntityClear()
{
	bot.clear();
	bot_queue.clear();
	for (unsigned f = 0; f < fx_list.size(); f++)
		delete fx_list[f];
  for (unsigned f = 0; f < device_list.size (); f++)
    delete device_list[f];
  for (int i = 0; i < MAX_PROJECTILES; i++)
    projectiles[i].Retire ();
	fx_list.clear();
  device_list.clear ();
}

Robot* EntityRobotFromId (int id)
{
  for (unsigned b = 0; b < bot.size (); b++) {
    if (bot[b].Id () == id)
      return &bot[b];
  }
  return NULL;
}


void EntityUpdate()
{
	in_update = true;
  for (int i = 0; i < MAX_PROJECTILES; i++)
    projectiles[i].Update ();
	//Last update, we queued up any newly added robots to avoid adding to the
	//list while we were iterating over it. Now put them in play.
	for (unsigned i = 0; i < bot_queue.size(); i++)
		bot.push_back(bot_queue[i]);
	bot_queue.clear();
	//process general fx
	for (unsigned f = 0; f < fx_list.size(); f++) {
    if (fx_list[f]->Active ()) {
      fx_list[f]->Update ();
    } else {
			delete fx_list[f];
			fx_list.erase(fx_list.begin() + f);
		}
	}
  //Process doors.
  for (unsigned f = 0; f < device_list.size (); f++) {
    device_list[f]->Update ();
  }
	//Drop dead bots
	for (unsigned b = 0; b < bot.size(); b++) {
		if (bot[b].Retired ()) {
			bot.erase (bot.begin () + b);
			break; ///Only delete one per frame, to avoid hitching during mass deaths.
		}
	}
	is_calm = true;
  int     active_counter = 0;

	dead_robots = 0;
	for (unsigned i = 0; i < bot.size(); i++) {
		//Things are not calm if any bots are actively trying to kill the player.
    if (bot[i].IsAlerted ()) {
      is_calm = false;
    }
		if (bot[i].Dead ())
			dead_robots++;
    if (bot[i].IsAlerted () && !bot[i].Dead ())
      active_counter++;
	}
	int	begin = SystemTick ();
	int	end = begin + 3;
	int	update_count = 0;
	static int	update_bot;
	while (SystemTick () < end && update_count < bot.size ()) {
		update_bot %= bot.size ();
		bot[update_bot].Update ();
		update_bot++;
		update_count++;
	}
	do_shoving();
  active_robots = active_counter;
	in_update = false;
}

void EntityRenderRobots (bool hidden)
{
	if (hidden) {
		for (unsigned i = 0; i < bot.size (); i++)
			bot[i].RenderHidden ();
	} else {
		for (unsigned i = 0; i < bot.size (); i++)
			bot[i].RenderBody ();
		RenderQuads ();
		RenderTriangles ();
		for (unsigned i = 0; i < bot.size (); i++)
			bot[i].RenderEye ();
		RenderQuads ();
		glDepthFunc (GL_EQUAL);
		for (unsigned i = 0; i < bot.size (); i++)
			bot[i].RenderIris ();
		RenderQuads ();
		for (unsigned i = 0; i < bot.size (); i++)
			bot[i].RenderPain ();
		glDepthFunc (GL_LEQUAL);
	}
}

void EntityRenderFx()
{
	glDepthMask(false);
	for (unsigned f = 0; f < fx_list.size(); f++)
		fx_list[f]->Render();
  for (int i = 0; i < MAX_PROJECTILES; i++)
    projectiles[i].Render ();
}