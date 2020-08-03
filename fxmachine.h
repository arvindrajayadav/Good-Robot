#ifndef MACHINE_H
#define MACHINE_H

#include "XMLDoc.h"
#include "fx.h"

struct MachinePart
{
	GLvector2             origin;       //The absolute origin of this sprite, regardless of where it travels.
	GLvector2             position;     //The current position of this sprite, as animated.
	GLvector2             move;
	GLrgba                color;
	GLrgba                sprite_color;
	SpriteEntry           sprite;
	bool									destroyed;
	bool                  glow;
	bool                  move_linear;
	bool                  is_moving;
	bool                  looping_movement;
	bool									indestructible;
	float                 depth;
	float                 size;
	float                 angle;
	float                 start_angle;
	float                 spin;
	int                   move_time;
	vector<float>         blink;
};

enum MachineUse
{
	MACHINE_USE_NONE,
	MACHINE_USE_INSURANCE,
	MACHINE_USE_GUNS,
	MACHINE_USE_FACTORY,
	MACHINE_USE_UPGRADE,
	MACHINE_USE_HATS,
  MACHINE_USE_SPAWN,
};

enum MachineMount
{
	MACHINE_MOUNT_FLOOR,
	MACHINE_MOUNT_CEILING,
	MACHINE_MOUNT_LEFT,
	MACHINE_MOUNT_RIGHT,
};

class MachineInfo
{
	friend class            fxMachine;

	vector<MachinePart>     _parts;           ///The list of individual animated sprites that compose the machine.
	GLvector2               _offset;          ///Used to move the entire machine relative to origin, so make adjusting things easier on the artist so they don't need to edit parts individually.
	MachineUse              _use;             ///What the machine does, gameplay-wise.  (If anything.)
	MachineMount            _mount;           ///How this unit is placed in the level.
	string                  _name;            ///The name given by  the artist, referenced in levels file.
	string                  _message;         ///Text shown to player when prompting them to activate the device.
	vector<float>           _blink;           ///Dictates the sprite brightness changing over time.
	GLbbox2                 _bbox;            ///The bounding box of this object, ignoring the bits that should be embedded into level geometry.
	GLcoord2                _cell_size;       ///How many cells wide and tall this machine is.
	int                     _max_active_robots; ///If this machine spawns robots, how many can I spawn at once before it stops?
  float                   _activate_radius; ///How close the player needs to be to interact with this.
  bool                    _can_activate;    ///If the player can interact with this.
  bool                    _shootable;       ///If bullets can hit it.
  bool                    _invulnerable;    ///If shootable, is it immune to damage.
  int                     _hitpoints;       ///If shootable, how much damage it can take before being destroyed.
  string                  _drop;            ///The name of the drop from the loot tables, to be created when machine dies.

	string                  StringFromXML(class rapidxml::xml_node<char>* node, string field);
	MachineUse              UseFromString(string val);
	MachineMount            MountFromString(string val);
	vector<float>           BlinkFromString(string s);

public:
	void                    Load (class rapidxml::xml_node<char>* node);
	MachineMount            Mount() const { return _mount; }
	string                  Name() const { return _name; }
	GLcoord2                Size() const { return _cell_size; }
	MachineUse              Use() const { return _use; }
};

class fxMachine : public fxDevice
{
	const MachineInfo*    _info;          ///The template machine from which this one is derived.
	vector<MachinePart>   _parts;         ///List of all machine parts and their current state.
	GLbbox2               _bbox;          ///Used for detecting collision with the machine.
	unsigned              _tick_offset;   ///Randomly perturbed value to keep machines from being in perfect sync.
	GLcoord2              _page;          ///The location of the page this machine occupies. Used for communicating with the zone.
	GLvector2             _spawner;       ///When spawning the player, they will appear here.
	GLvector2             _dropoff;       ///When spawning stuff, it will appear here.
	unsigned              _factory_next;  ///The next game tick when we can pop out a bot.
	bool                  _on;            ///If false, the machine should stop doing whatever it does.
  bool                  _destroyed;     ///If it's been blown up by gunfire.
	bool									_humming;				///If the machine is humming because the player is close.
  int                   _hitpoints;     ///Machine is destroyed when HP hits zero.

	void                  FactoryUpdate();
	void                  TurnOff();
  void                  TurnOn ();

public:
	void                  Init(GLvector2 position, GLcoord2 page, const MachineInfo* info);
	void									Destroy ();
	bool                  Collide(GLvector2 pos);
	void                  Hit(GLvector2 pos, int damage);
	void                  Render();
	void                  RenderOccluded();
	void                  Update();
	MachineUse            Use() { return _info->_use; }
	fxType                Type() { return FX_MACHINE; }
};

#endif // MACHINE_H