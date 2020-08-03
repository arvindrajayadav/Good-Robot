/*-----------------------------------------------------------------------------

Machine.cpp

Good Robot
(c) 2015 Pyrodactyl

This manages the complex vending machine objects that can be spread around
the gameworld. This is actually inherited from an fx object, just like bullets
and pickups, but this one is a bit too large and complex to end up shoved in
the kid's table in fx.cpp.

-----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "camera.h"
#include "collision.h"
#include "drop.h"
#include "entity.h"
#include "fx.h"
#include "loaders.h"
#include "fxmachine.h"
#include "menu.h"
#include "particle.h"
#include "player.h"
#include "random.h"
#include "render.h"
#include "world.h"
#include "zone.h"

#define SPAWN_RATE      200
#define LONG_COOLDOWN   4000

using namespace pyrodactyl;

/*-----------------------------------------------------------------------------
MachineInfo class holds the properties of machine types.
-----------------------------------------------------------------------------*/

string MachineInfo::StringFromXML(rapidxml::xml_node<char> *node, string field)
{
	string result;

	LoadStr(result, field, node);
	return result;
}

vector<float> MachineInfo::BlinkFromString(string s)
{
	vector<float>   b;

	for (unsigned i = 0; i < s.length(); i++) {
		b.push_back((float)stoi(s.substr(i, 1), nullptr, 16) / 15.0f);
	}

	return b;
}

MachineUse MachineInfo::UseFromString(string input)
{
	input = StringToLower(input);
	if (input.compare("insurance") == 0)
		return MACHINE_USE_INSURANCE;
	if (input.compare("shop") == 0)
		return MACHINE_USE_GUNS;
	if (input.compare("factory") == 0)
		return MACHINE_USE_FACTORY;
	if (input.compare("upgrade") == 0)
		return MACHINE_USE_UPGRADE;
	if (input.compare ("hats") == 0)
		return MACHINE_USE_HATS;
	if (input.compare ("spawn") == 0)
		return MACHINE_USE_SPAWN;
	return MACHINE_USE_NONE;
}

MachineMount MachineInfo::MountFromString(string input)
{
	input = StringToLower(input);
	if (input.compare("ceiling") == 0)
		return MACHINE_MOUNT_CEILING;
	if (input.compare("left") == 0)
		return MACHINE_MOUNT_LEFT;
	if (input.compare("right") == 0)
		return MACHINE_MOUNT_RIGHT;
	return MACHINE_MOUNT_FLOOR;
}

void MachineInfo::Load(class rapidxml::xml_node<char>* node)
{
	float group_rotate;

	_name = node->name();
	_message = StringFromXML(node, "Message");
	_drop = StringFromXML(node, "Drop");
	_use = UseFromString(StringFromXML(node, "Use"));
	_mount = MountFromString(StringFromXML(node, "Mount"));
	group_rotate = StringToFloat(StringFromXML(node, "Angle"));
	_hitpoints = EnvGetHitpointsFromID(StringFromXML(node, "Hitpoints"));
	//Negative hitpoints means a machine is immune to damage.
	_invulnerable = (_hitpoints <= 0);
	//Hitpoints of zero means a machine can't be hit with bullets.
	_shootable = (_hitpoints != 0);
	_offset = StringToVector2(StringFromXML(node, "Offset"));
	_offset = _offset.Rotated(group_rotate);
	_activate_radius = StringToFloat(StringFromXML(node, "ActivateRadius"));
	_can_activate = false;
	if (!LoadNum(_max_active_robots, "MaxActiveRobots", node))
		_max_active_robots = 6;

	_parts.clear();
	_bbox.Clear();
	for (int part_count = 0;; part_count++) {
		rapidxml::xml_node<char> *part_node;
		MachinePart               mp;

		part_node = node->first_node(StringSprintf("Part%d", part_count).c_str());
		if (!NodeValid(part_node))
			break;
		mp.origin.x = StringToFloat(StringFromXML(part_node, "X"));
		mp.origin.y = StringToFloat(StringFromXML(part_node, "Y"));
		mp.origin = mp.origin.Rotated(group_rotate);
		mp.origin += _offset;
		mp.position = mp.origin;
		mp.depth = StringToFloat(StringFromXML(part_node, "Depth"));
		mp.color = GLrgbaFromHex(StringFromXML(part_node, "Color"));
		mp.size = StringToFloat(StringFromXML(part_node, "Size"));
		mp.sprite = SpriteEntryLookup(StringFromXML(part_node, "Sprite"));
		mp.start_angle = StringToFloat(StringFromXML(part_node, "Angle")) + group_rotate;
		mp.spin = StringToFloat(StringFromXML(part_node, "Spin"));
		mp.blink = BlinkFromString(StringFromXML(part_node, "Blink"));
		mp.indestructible = (bool)(StringToInt (StringFromXML (part_node, "Indestructible")) != 0);
		mp.glow = (bool)(StringToInt (StringFromXML (part_node, "Glow")) != 0);
		mp.destroyed = false;

		//Move also needs to rotate depending on the rotate value
		mp.move = StringToVector2(StringFromXML(part_node, "Move"));
		mp.move = mp.move.Rotated(group_rotate);

		int time = StringToInt(StringFromXML(part_node, "MoveTime"));
		mp.move_linear = time > 0;
		mp.move_time = max(abs(time), 1);//Must be at least 1, or else divide by zero.
		mp.is_moving = mp.move != GLvector2();
		mp.looping_movement = StringToInt(StringFromXML(part_node, "MoveLoop")) != 0;
		_parts.push_back(mp);
		if (!mp.glow) {
			_bbox.ContainPoint(mp.position - (mp.size / 2.0f));
			_bbox.ContainPoint(mp.position + (mp.size / 2.0f));
		}
	}
	if (_use == MACHINE_USE_GUNS || _use == MACHINE_USE_UPGRADE || _use == MACHINE_USE_HATS)
		_can_activate = true;

	//Now shave off the portion of the bbox that will be embedded into the level geometry.
	if (_mount == MACHINE_MOUNT_FLOOR)
		_bbox.pmax.y = 0.0f;
	if (_mount == MACHINE_MOUNT_LEFT)
		_bbox.pmin.x = 0.0f;
	_cell_size.x = max((int)ceil(_bbox.Size().x), 1);
	_cell_size.y = max((int)ceil(_bbox.Size().y), 1);
}

/*-----------------------------------------------------------------------------
This sort is used to put machine parts in order so they can be rendered
back-to-front.
-----------------------------------------------------------------------------*/

static int compare_parts(const void* part1, const void* part2)
{
	const MachinePart*  p1 = (MachinePart*)part1;
	const MachinePart*  p2 = (MachinePart*)part2;

	if (p1->depth < p2->depth)
		return -1;
	else if (p1->depth > p2->depth)
		return 1;
	return 1;
}

/*-----------------------------------------------------------------------------
fxMachine holds the instance of a single machine within the game. It's
in charge of animating, rendering, and activating it.
-----------------------------------------------------------------------------*/

void fxMachine::Init(GLvector2 position, GLcoord2 page, const MachineInfo* info)
{
	_origin = position;
	_info = info;
	_page = page;
	_active = true;
	_tick_offset = RandomVal();
	_bbox.Clear();
	_bbox.ContainPoint(_origin);
	_factory_next = 0;
	_on = true;
	_spawner = position;
	_hitpoints = _info->_hitpoints;
	_destroyed = false;
	_humming = false;
	_dropoff = _bbox.Center ();
	//See if we're on the left or right half of this page. (Room.)
	//We want to spawn stuff TOWARDS the center.
	float page_x = fmod (position.x, (float)PAGE_SIZE);
	float offset = _bbox.Size ().x + 1.0f;
	if (page_x > PAGE_HALF)
		_dropoff.x -= offset;
	else
		_dropoff.x += offset;
	if (_info->Use () == MACHINE_USE_FACTORY) 
		WorldZone ()->PageGet (_page)->FactoryAdded ();
	for (unsigned i = 0; i < _info->_parts.size(); i++) {
		MachinePart   p;

		p = _info->_parts[i];
		p.depth += DEPTH_MACHINES;
		p.sprite_color = p.color;
		p.angle = p.start_angle;
		p.destroyed = false;
		if (!p.glow) {
			_bbox.ContainPoint(_origin + p.origin + (p.size / 2));
			_bbox.ContainPoint(_origin + p.origin - (p.size / 2));
		}
		_parts.push_back(p);
	}
	//If this thing has an activation radius, make sure the bbox is large enough to contain it.
	if (info->_activate_radius > 0.0f) {
		_bbox.ContainPoint(_origin + GLvector2(info->_activate_radius, info->_activate_radius));
		_bbox.ContainPoint(_origin - GLvector2(info->_activate_radius, info->_activate_radius));
	}
	//Don't know why anyone would make a machine with no parts, but let's avoid crashing anyway.
	if (!_parts.empty()) {
		//Things should pop out of the center of the first sprite.
		_spawner = _parts[0].position;
		qsort(&_parts[0], _parts.size(), sizeof(MachinePart), compare_parts);
	}
}

void fxMachine::Destroy ()
{
	if (_destroyed)
		return;
	_destroyed = true;
	if (_info->Use () == MACHINE_USE_FACTORY)
		WorldZone ()->PageGet (_page)->FactoryDestroyed ();
	//Make an explosion to cover up the vanising device.
	for (unsigned i = 0; i < _info->_parts.size (); i++) {
		MachinePart*    p;
		p = &_parts[i];
		if (!p->indestructible)
			p->destroyed = true;
		if (!p->glow && !p->indestructible) {
			fxExplosion* e = new fxExplosion;
			e->Init (OWNER_NONE, _origin + p->position, 1, p->size / 1.4f);
			EntityFxAdd (e);
			ParticleDebris (_origin + p->position, p->size / 4.0f, 3, 1.0f + RandomFloat () * 7.0f);
		}
	}
	//Drop stuff.
	if (!_info->_drop.empty ())
		DropLoot (_info->_drop, _origin);
}

void fxMachine::Update()
{
	MachinePart*    p;
	unsigned        blink_tick;
	unsigned        now;

	now = GameTick();
	if (_destroyed)
		return;
	if (!_info->_invulnerable && _hitpoints <= 0) {
		Destroy ();
		return;
	}
	if (_info->_use == MACHINE_USE_SPAWN) {
		if (Player()->Warranty() && !_on)
			TurnOn();
		if (!Player()->Warranty() && _on && !PlayerIgnore())
			TurnOff();
	}
	if (_on) {
		blink_tick = GameTick() / 20 + _tick_offset;
		for (unsigned i = 0; i < _info->_parts.size(); i++) {
			p = &_parts[i];
			if (!p->blink.empty()) {
				int index = (blink_tick) % p->blink.size();
				p->sprite_color = p->color * p->blink[index];
			}
			if (p->is_moving) {
				float delta = (float)(now % p->move_time) / (float)p->move_time;

				//Scale delta so it runs from 0 to 1 and back to 0 again.
				if (p->looping_movement) {
					delta *= 2.0f;
					if (delta > 1.0f)
						delta = 2.0f - delta;
				}
				//Use delta to move this object.
				if (!p->move_linear)
					delta = syMathScalarCurve(delta);
				p->position = p->origin + p->move * delta;
			}
			p->angle += p->spin;
		}
	}
	if (_info->_can_activate) {
		bool    close = false;
		float   distance = GLvector2 (_origin - PlayerOrigin ()).Length ();

		if (distance < _info->_activate_radius)
			close = true;
		if (close) {
			float volume = 1.0f - (distance / _info->_activate_radius);
			PlayerOfferAccess (0.0f, _info->_message.c_str ());
			WorldVendingNear ();
			_humming = true;
			if (PlayerAccessActivated () && !MenuIsOpen ()) {
				WorldItemDropoffSet (_dropoff);
				PlayerOriginSet (_dropoff);
				if (_info->_use == MACHINE_USE_GUNS)
					MenuOpen (MENU_STORE);
				if (_info->_use == MACHINE_USE_UPGRADE)
					MenuOpen (MENU_UPGRADE);
				if (_info->_use == MACHINE_USE_HATS)
					MenuOpen (MENU_HAT);
			}
		} else if (_humming) { //If we're humming but player is no longer close.
			_humming = false;
			AudioLoop (LOOP_ACCESS, 1.0f, 0.0f);
		}
	}
	if (_info->_use == MACHINE_USE_FACTORY)
		FactoryUpdate();
}

void fxMachine::TurnOff() { _on = false; }

void fxMachine::TurnOn() { _on = true; }

void fxMachine::FactoryUpdate()
{
	//Limit the rate at which we spawn robots.
	if (GameTick() < _factory_next)
		return;
	_factory_next = GameTick() + SPAWN_RATE;

	int           robot_id;
	Robot         b;
	GLvector2     launch_vector;
	bool          player_can_see_us;
	GLvector2     to_player;

	//Take our origin, nudge it slightly towards the player, and see if that point
	//ends up on-screen. If so, they can see us well enough.
	to_player = GLvector2(PlayerPosition() - _origin).Normalized() * 0.5f;
	player_can_see_us = RenderPointVisible(_origin - to_player);
	//Make sure the spawner is on-screen, so we're not spewing out robots in parts of the level
	//where they don't matter.
	if (!player_can_see_us)
		return;
	//If there are no robots left (or never were any to begin with)
	//then shut down this machine.
	if (WorldZone()->RobotSpawnCount(_page) == 0) {
		if (_on)
			TurnOff();
		ParticleSmoke (_origin + _spawner, 0.85f, 2);
		_factory_next += 100;
		return;
	}
	//If the population is too high, put this spawner on a long cooldown
	//before we try again.
	if (EntityRobotsActive() > _info->_max_active_robots) {
		_factory_next += LONG_COOLDOWN;
		return;
	}
	//Don't make robots unless there's a clear line of sight to the player,
	//to avoid spawning robots or behind walls.
	if (!CollisionLos(_origin + _parts[0].position, PlayerPosition(), 0.5f))
		return;
	robot_id = WorldZone()->RobotSpawnId(_page);
	//If RobotSpawnId returns ROBOT_INVALID, then the current location is
	//out of robots.
	if (robot_id == ROBOT_INVALID) {
		if (_on)
			TurnOff();
		ParticleSmoke(_origin + _spawner, 0.85f, 2);
		_factory_next += 100;
		return;
	}
	//Spawn a robot
	launch_vector = GLvectorFromAngle(_parts[0].angle + 180.0f);
	b.Init(_bbox.Center(), robot_id);
	b.Launch(launch_vector);
	EntityRobotAdd(b);
	//Make some particle effects to cover the spawn.
	ParticleBloom (_bbox.Center (), GLrgba (1, 1, 1), 1.0f, 500);
	AudioPlay ("skate", _bbox.Center ());
}

void fxMachine::Render()
{
	MachinePart*        p;

	if (_on) {
		for (unsigned i = 0; i < _info->_parts.size(); i++) {
			p = &_parts[i];
			if (p->destroyed)
				continue;
			RenderQuad (_origin + p->position, p->sprite, p->sprite_color, p->size, p->angle, p->depth, p->glow);
		}
	}	else {
		for (unsigned i = 0; i < _info->_parts.size(); i++) {
			p = &_parts[i];
			if (p->glow || p->destroyed)
				continue;
			RenderQuad(_origin + p->position, p->sprite, GLrgba(), p->size, p->angle, p->depth, p->glow);
		}
	}
	if (EnvValueb(ENV_BBOX)) {
		if (_destroyed)
			glColor3f(1, 0, 0);
		else
			glColor3f (0, 1, 0);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_TEXTURE_2D);
		_bbox.Render();
		glEnable(GL_TEXTURE_2D);
	}
}

void fxMachine::RenderOccluded()
{
	MachinePart*    p;

	for (unsigned i = 0; i < _info->_parts.size(); i++) {
		p = &_parts[i];
		if (p->glow || p->destroyed)
			continue;
		RenderQuad(_origin + p->position, p->sprite, GLrgba(), p->size, p->angle, p->depth, p->glow);
	}
}

bool fxMachine::Collide(GLvector2 bullet_pos)
{
	if (!_info->_shootable)
		return false;
	if (_destroyed)
		return false;
	//If a factory runs out of robots, you can't shoot it.
	if (_info->_use == MACHINE_USE_FACTORY && !_on)
		return false;
	if (!_bbox.Contains(bullet_pos))
		return false;
	MachinePart*    p;
	SpriteUnit      s;

	for (unsigned i = 0; i < _info->_parts.size(); i++) {
		p = &_parts[i];
		//Bullets can't hit glowing parts, which are usually light auras and other non-tangible stuff.
		if (p->glow)
			continue;
		s.Init(p->sprite, p->size, GLrgba());
		s.Move(_origin + p->position, p->angle);
		if (s.Collide(bullet_pos))
			return true;
	}
	return false;
}

void fxMachine::Hit(GLvector2 x, int damage)
{
	if (_info->_invulnerable)
		return;
	_hitpoints -= damage;
}