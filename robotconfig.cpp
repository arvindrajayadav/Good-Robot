/*-----------------------------------------------------------------------------

RobotConfig.cpp

The RobotConfig class reads in and holds the data for the various robot types.
If you're looking for the individual robots themselves, you want Robot.cpp.

Good Robot
(c) 2015 Pyrodactyl

-----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "bodyparts.h"
#include "env.h"
#include "ini.h"
#include "robot.h"
#include "sprite.h"

#define DEFAULT_ATTACK_RANGE    3
#define ROBOT_SPEED_SCALING     0.01f

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

eBodyClass RobotConfig::BodyFromString(string str)
{
	eBodyClass bc;

	bc = RBODY_FIXED;
	str = StringToLower(str);
	if (str == "snake")
		bc = RBODY_SNAKE;
	if (str == "squid")
		bc = RBODY_SQUID;
	if (str == "worm")
		bc = RBODY_WORM;
	if (str == "jelly")
		bc = RBODY_JELLY;
	if (str == "turret")
		bc = RBODY_TURRET;
	return bc;
}

void RobotConfig::PartsFromString(string str, SpriteEntry* parts, bool boss)
{
	unsigned        i;
	SpriteEntry     val;
	SpriteEntry     base;
	vector<string>  list;

	list = StringSplit(str, " ,");
	base = SPRITE_ROBOT0;
	//if (boss)
	//base = SPRITE_BOSS0;
	for (i = 0; i < MAX_PARTS; i++)
		parts[i] = SPRITE_INVALID;
	i = 0;
	val = SPRITE_INVALID;
	while (i < list.size() && i < MAX_PARTS) {
		int     index;

		index = atoi(list[i].c_str());
		//If atoi returns zero, then this is probably a text name of the sprite.
		if (index == 0)
			index = (int)SpriteEntryLookup(list[i].c_str());
		if (index == -1)
			val = SPRITE_INVALID;
		else
			val = (SpriteEntry)(base + index);
		parts[i] = val;
		if (val == SPRITE_INVALID)
			break;
		i++;
	}
	part_count = i;
	//The rest of the list is empty.
	while (i < MAX_PARTS) {
		parts[i] = SPRITE_INVALID;
		i++;
	}
}

AiCore RobotConfig::CoreFromString(string str)
{
	if (str == "beeline")		return AI_BEELINE;
	if (str == "tunnel")		return AI_TUNNEL;
	if (str == "walk")			return AI_WALK;
	if (str == "pounce")		return AI_POUNCE;
	if (str == "orbit")			return AI_ORBIT;
	if (str == "sentry")		return AI_SENTRY;
	if (str == "hitnrun")		return AI_HITNRUN;
	if (str == "guard")			return AI_GUARD;
	Console("Warning: Unknown AiCore '%s'.", str.c_str());
	return AI_BEELINE;
}

AiIdle RobotConfig::IdleFromString(string str)
{
	if (str == "wait")   return AI_IDLE_WAIT;
	if (str == "burrow") return AI_IDLE_BURROW;
	if (str == "hang")   return AI_IDLE_HANG;
	return AI_IDLE_WAIT;
}

eMeleeClass RobotConfig::MeleefromString(string str)
{
	eMeleeClass     mc;

	str = StringToLower(str);
	mc = MELEE_SHOVE;
	if (str == "shove")   mc = MELEE_SHOVE;
	if (str == "bite")    mc = MELEE_BITE;
	if (str == "explode") mc = MELEE_EXPLODE;
	return mc;
}

eEyeMove RobotConfig::EyeMoveFromString(string str)
{
	eEyeMove     em;

	str = StringToLower(str);
	em = EYEMOVE_FIXED;
	if (str == "sweep")    em = EYEMOVE_SWEEP;
	if (str == "scan")     em = EYEMOVE_SCAN;
	if (str == "player")   em = EYEMOVE_PLAYER;
	if (str == "heading")  em = EYEMOVE_HEADING;
	return em;
}

vector<int> RobotConfig::WeaponsFromString(string list)
{
	vector<string>  entries = StringSplit(list, " ,");
	vector<int>     id;

	for (int i = 0; i < entries.size(); i++) {
		id.push_back(EnvProjectileId(entries[i]));
	}
	return id;
}

Weapon RobotConfig::WeaponFromString(string data)
{
	Weapon          w;
	vector<string>  fields;

	fields = StringSplit(data, " ");
	w.projectile_id = 0;
	w.body_part = 0;
	w.cooldown = 100;
	w.next_fire = 0;
	w.volley = 0;
	if (fields.size() < 4)
		return w;
	//The first field might be the name of a projectile OR robot.
	//We save the text now and fill these fields in later once we have
	//all the data.
	w.payload_name = StringToLower(fields[0]);
	w.robot_id = 0;
	w.projectile_id = 0;
	w.body_part = StringToInt(fields[1]);
	w.volley = max(1, StringToInt(fields[2]));
	w.cooldown = StringToInt(fields[3]);
	return w;
}

bool RobotConfig::WeakpointFromString(string data, Weakpoint& w)
{
	vector<string>  fields;

	fields = StringSplit(data, " ,");
	if (fields.size() < 5)
		return false;
	w.position.x = StringToFloat(fields[0]);
	w.position.y = StringToFloat(fields[1]);
	w.color = GLrgbaFromHex(fields[2]);
	w.sprite = (SpriteEntry)(SPRITE_ROBOT0 + StringToInt(fields[3]));
	w.size = StringToFloat(fields[4]);
	return true;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void RobotConfig::EyeFromString(string data)
{
	vector<string> eye_parts = StringSplit(data, " ,");
	if (eye_parts.size() > 1) {
		eye = SpriteEntryLookup(eye_parts[0]);
		iris = SpriteEntryLookup(eye_parts[1]);
	}
	else if (!eye_parts.empty()) { //This is just to support old system.
		int index = StringToInt(data);
		eye = (SpriteEntry)(index + SPRITE_EYE0);
		iris = (SpriteEntry)(index + SPRITE_IRIS0);
	}
	else {
		eye = iris = SPRITE_INVALID;
	}
}

void RobotConfig::VoiceFromInt(int sound_group)
{
	switch (sound_group) {
	case -1: sound_see = "boss1_see"; sound_hit = "boss1_hit"; sound_die = "boss1_die"; break;
	case -2: sound_see = "boss2_see"; sound_hit = "boss2_hit"; sound_die = "boss2_die"; break;
	case 1:  sound_see = "voice1_see"; sound_hit = "voice1_hit"; sound_die = "voice1_die"; break;
	case 2:  sound_see = "voice2_see"; sound_hit = "voice2_hit"; sound_die = "voice2_die"; break;
	case 3:  sound_see = "voice3_see"; sound_hit = "voice3_hit"; sound_die = "voice3_die"; break;
	case 4:  sound_see = "voice4_see"; sound_hit = "voice4_hit"; sound_die = "voice4_die"; break;
	case 5:  sound_see = "voice5_see"; sound_hit = "voice5_hit"; sound_die = "voice5_die"; break;
	case 6:  sound_see = "voice6_see"; sound_hit = "voice6_hit"; sound_die = "voice6_die"; break;
	default: sound_see = "coin";  sound_hit = "coin"; sound_die = "coin"; break;
	}
}

void RobotConfig::LoadTemplate(iniFile &ini, string section)
{
	int     keys;
	string  val;
	string  key;

	keys = ini.SectionKeys(section);
	for (int k = 0; k < keys; k++) {
		key = StringToLower(ini.SectionKey(section, k));
		val = ini.SectionValue(section, k);
		if (!key.compare("name"))
			name = val;
		else if (!key.compare("drop"))
			drop = val;
		else if (!key.compare("bodytype"))
			body = BodyFromString(val);
		else if (!key.compare("bodyparts"))
			PartsFromString(val, parts, is_boss);
		else if (!key.compare("aicore"))
			ai_core = CoreFromString(val);
		else if (!key.compare("aiidle"))
			ai_idle = IdleFromString(val);
		else if (!key.compare("eyemovement"))
			eye_movement = EyeMoveFromString(val);
		else if (!key.compare("meleeclass"))
			melee_class = MeleefromString(val);
		else if (!key.compare("bodycolor"))
			body_color = GLrgbaFromHex(val.c_str());
		else if (!key.compare("eye"))
			EyeFromString(val);
		else if (!key.compare("eyecolor"))
			eye_color = GLrgbaFromHex(val.c_str());
		else if (!key.compare("eyeoffset"))
			eye_offset = StringToVector2(val);
		else if (!key.compare("eyesize"))
			eye_size = StringToFloat(val);
		//Ints
		else if (!key.compare("hitpoints"))
			hitpoints = EnvGetHitpointsFromID(val);
		else if (!key.compare("score"))
			score_value = EnvGetScoreFromID(val);
		else if (!key.compare("xp"))
			xp_value = EnvGetMoneyFromID(val);
		else if (!key.compare("maxchildren"))
			max_children = StringToInt(val);
		else if (!key.compare("meleedamage"))
			melee_damage = StringToInt(val);
		else if (!key.compare("legs"))
			legs = StringToInt(val);
		else if (!key.compare("armor"))
			armor = StringToInt(val);
		else if (!key.compare("voice"))
			VoiceFromInt(StringToInt(val));
		else if (!key.compare("childdeathdamage"))
			child_death_damage = StringToInt(val);

		//Floats
		else if (!key.compare("screenshakealert"))
			screen_shake_alert = StringToFloat(val);
		else if (!key.compare("screenshakebore"))
			screen_shake_bore = StringToFloat(val);
		else if (!key.compare("attackrange"))
			attack_range = StringToFloat(val);
		else if (!key.compare("shovepower"))
			melee_shove_power = StringToFloat(val);
		else if (!key.compare("walkstride"))
			walk_stride = StringToFloat(val);
		else if (!key.compare("walkheight"))
			walk_height = StringToFloat(val);
		else if (!key.compare("walkstand"))
			walk_stand = StringToFloat(val);
		else if (!key.compare("walkcrouch"))
			walk_crouch = StringToFloat(val);
		else if (!key.compare("spotdistance"))
			spot_distance = StringToFloat(val);
		else if (!key.compare("bodysize"))
			size = StringToFloat(val);
		else if (!key.compare("inertia"))
			inertia = StringToFloat(val);
		else if (!key.compare("speed"))
			speed = StringToFloat(val) * ROBOT_SPEED_SCALING;

		//Bools
		else if (!key.compare("dependant"))
			is_dependant = StringToInt(val) != 0;
		else if (!key.compare("boss"))
			is_boss = StringToInt(val) != 0;
		else if (!key.compare("finalboss"))
			is_final_boss = StringToInt(val) != 0;
		else if (!key.compare("attackpredict"))
			attack_predict = StringToInt(val) != 0;
		else if (!key.compare("follower"))
			is_follower = StringToInt(val) != 0;

		//If the file defines ANY weapons, it must define all of them.
		else if (!key.compare("weapon1"))
			weapons.clear();

		else if (!key.compare("proximitywarning")) {
			proximity_warning = val;
			warn_proximity = true;
		}
	}
}

void RobotConfig::Load(iniFile &ini, string section)
{
	RobotConfig r;
	int         template_id;

	string      temp;

	template_id = ROBOT_INVALID;
	max_children = 0;
	score_value = 0;
	xp_value = 0;
	melee_damage = 0;
	hitpoints = 1;
	child_death_damage = 0;
	armor = 0;
	attack_predict = 0;
	attack_range = 1.0f;
	refire_melee = 100;
	melee_shove_power = 0.0f;
	eye_movement = 0;
	legs = 0;
	is_boss = false;
	has_weakpoints = false;
	warn_proximity = false;
	is_explosive = false;
	is_dependant = false;
	is_follower = false;
	is_invulnerable = false;
	eye_size = 1.0f;
	walk_stride = 1.0f;
	walk_height = 1.0f;
	walk_crouch = 1.0f;
	walk_stand = 1.0f;
	size = 1.0f;
	speed = 0.0f;
	spot_distance = 1.0f;
	drop = "";
	screen_shake_alert = screen_shake_bore = 0.0f;
	weapons.clear();
	weakpoint.clear();
	weapon_drops.clear();


	temp = StringToLower(ini.StringGet(section, "Template"));
	if (!temp.empty())
		template_id = EnvRobotIndexFromName(temp);
	if (template_id != ROBOT_INVALID) {
		const RobotConfig* rc;

		rc = EnvRobotConfig(template_id);
		*this = *rc;
		LoadTemplate(ini, section);
	} else {
		name = ini.StringGet(section, "Name");
		drop = ini.StringGet(section, "Drop");
		proximity_warning = ini.StringGet(section, "ProximityWarning");
		warn_proximity = false;
		if (proximity_warning.size() > 1) {
			warn_proximity = true;
		}
		body = BodyFromString(ini.StringGet(section, "BodyType"));
		is_boss = ini.IntGet(section, "Boss") != 0;
		is_final_boss = ini.IntGet(section, "FinalBoss") != 0;
		is_dependant = ini.IntGet(section, "Dependant") != 0;
		is_explosive = ini.IntGet(section, "Explosive") != 0;
		is_follower = ini.IntGet(section, "Follower") != 0;
		PartsFromString(ini.StringGet(section, "BodyParts"), parts, is_boss);
		ai_core = CoreFromString(ini.StringGet(section, "AiCore"));
		ai_idle = IdleFromString(ini.StringGet(section, "AiIdle"));
		eye_movement = EyeMoveFromString(ini.StringGet(section, "EyeMovement"));
		melee_class = MeleefromString(ini.StringGet(section, "MeleeClass"));
		body_color = GLrgbaFromHex(ini.StringGet(section, "BodyColor").c_str());
		EyeFromString(ini.StringGet(section, "Eye"));
		eye_color = GLrgbaFromHex(ini.StringGet(section, "EyeColor").c_str());
		eye_offset = ini.Vector2Get(section, "EyeOffset");
		eye_size = ini.FloatGet(section, "EyeSize");
		hitpoints = EnvGetHitpointsFromID(ini.StringGet(section, "HitPoints"));
		child_death_damage = ini.IntGet(section, "ChildDeathDamage");
		score_value = EnvGetScoreFromID(ini.StringGet(section, "Score"));
		xp_value = EnvGetMoneyFromID(ini.StringGet(section, "Xp"));
		max_children = ini.IntGet(section, "MaxChildren");

		screen_shake_alert = ini.FloatGet(section, "ScreenShakeAlert");
		screen_shake_bore = ini.FloatGet(section, "ScreenShakeBore");

		//A robot can never reduce score
		if (score_value <= 0) {
			score_value = 0;
		}
		melee_damage = ini.IntGet(section, "MeleeDamage");
		attack_predict = ini.IntGet(section, "AttackPredict");
		attack_range = ini.FloatGet(section, "AttackRange");
		if (attack_range == 0)
			attack_range = DEFAULT_ATTACK_RANGE;
		melee_shove_power = ini.FloatGet(section, "ShovePower");
		refire_melee = ini.IntGet(section, "RefireMelee");
		spot_distance = ini.FloatGet(section, "SpotDistance");
		if (spot_distance < 1)
			spot_distance = DEFAULT_SPOT_DISTANCE;
		//Isn't this deprecated?
		weapon_drops = WeaponsFromString(ini.StringGet(section, "DropWeapons"));
		inertia = ini.FloatGet(section, "Inertia");
		walk_stride = ini.FloatGet(section, "WalkStride");
		walk_height = ini.FloatGet(section, "WalkHeight");
		walk_stand = ini.FloatGet(section, "WalkStand");
		walk_crouch = ini.FloatGet(section, "WalkCrouch");
		armor = ini.IntGet(section, "Armor");
		legs = ini.IntGet(section, "Legs");
		size = ini.FloatGet(section, "BodySize");
		speed = ini.FloatGet(section, "Speed") * ROBOT_SPEED_SCALING;

		VoiceFromInt(ini.IntGet(section, "Voice"));
	}
	type = StringToLower(section);

	if (hitpoints < 0)
		is_invulnerable = true;

	//Get the list of weapons
	while (1) {
		string    data;

		data = ini.StringGet(section, StringSprintf("Weapon%lu", weapons.size() + 1));
		if (data.length() < 2)
			break;
		weapons.push_back(WeaponFromString(data));
	}
	//Get the list of optional weak points.
	has_weakpoints = false;
	while (1) {
		Weakpoint w;
		char      entry[16];

		sprintf(entry, "Weakpoint%lu", weakpoint.size() + 1);
		if (!WeakpointFromString(ini.StringGet(section, entry), w))
			break;
		has_weakpoints = true;
		//We want the sprite placement to be in proportion to the body.
		w.position *= size;
		weakpoint.push_back(w);
	}
}