/*-----------------------------------------------------------------------------

Character.cpp

This loads and tracks the properties of the various playable characters.

Good Robot
(c) 2015 Pyrodactyl

-----------------------------------------------------------------------------*/

#include "master.h"

#include "character.h"
#include "ini.h"
#include "spritemap.h"

//We scale the numbers down so artists don't have to deal with annoying
//tiny values like 0.0001. Most artist-facing numbers should be around 1.0.
#define CHARACTER_SCALING   10.0f

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

int Character::AbilityFromName(string name)
{
	name = StringToLower(name);
	if (name.compare("magnet") == 0)
		return ABILITY_MAGNET;
	if (name.compare("scanner") == 0)
		return ABILITY_SCANNER;
	if (name.compare("compass") == 0)
		return ABILITY_COMPASS;
	/*if (name.compare("glide") == 0)
		return ABILITY_GLIDE;*/
	if (name.compare("target_laser") == 0)
		return ABILITY_TARGET_LASER;
	return ABILITY_INVALID;
}

void Character::Init(class iniFile f, string name)
{
	_name = name;
	_color = GLrgbaFromHex(f.StringGet(_name, "Color"));
	_size_torso = f.FloatGet(_name, "SizeTorso") / CHARACTER_SCALING;
	_size_head = f.FloatGet(_name, "SizeHead") / CHARACTER_SCALING;
	_size_eye = f.FloatGet(_name, "SizeEye");
	_size_launcher = f.FloatGet(_name, "SizeLauncher") / CHARACTER_SCALING;
	_size_laser = f.FloatGet(_name, "SizeLaser") / CHARACTER_SCALING;

	_pos_head = f.Vector2Get(_name, "PositionHead") / CHARACTER_SCALING;
	_pos_laser = f.Vector2Get(_name, "PositionLaser") / CHARACTER_SCALING;
	_pos_launcher = f.Vector2Get(_name, "PositionLauncher") / CHARACTER_SCALING;

	_eye_number = f.IntGet(_name, "EyeNumber");

	_sprite_torso = SpriteEntryLookup(f.StringGet(_name, "SpriteTorso"));
	_sprite_head = SpriteEntryLookup(f.StringGet(_name, "SpriteHead"));
	_sprite_launcher = SpriteEntryLookup(f.StringGet(_name, "SpriteLauncher"));
	_sprite_laser = SpriteEntryLookup(f.StringGet(_name, "SpriteLaser"));;

	_speed = f.FloatGet(_name, "Speed");

	vector<string> abilities = StringSplit(f.StringGet(_name, "Abilities"), " ,");
	for (unsigned i = 0; i < ABILITY_TYPES; i++)
		_ability[i] = false;
	for (unsigned i = 0; i < abilities.size(); i++) {
		int a = AbilityFromName(abilities[i]);
		if (a != ABILITY_INVALID)
			_ability[a] = true;
	}
}