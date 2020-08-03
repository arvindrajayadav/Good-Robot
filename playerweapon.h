#ifndef PLAYERWEAPON_H
#define PLAYERWEAPON_H

class PlayerWeapon
{

  bool                _firing;
	bool								_fired_this_frame;
  const Projectile*   _p_info;
  GLrgba              _weapon_glow;
  float               _weapon_warmup;
  float               _weapon_cooldown;
  float               _weapon_spin;
  float               _weapon_angle;
  int                 _cooldown_expires;
  int                 _shot_refire;
  int                 _shot_warmup;

  void                Fire (GLvector2 origin, GLvector2 shoot);


public:
  PlayerWeapon () { _p_info = NULL; }

  float               Angle () { return _weapon_angle; }
  GLrgba              Color () { return _weapon_glow; }
  float               Cooldown ();
  void                Equip (const Projectile* p_info);
  bool                Firing () { return _firing; }
	bool                Fired () { return _fired_this_frame; }
  const Projectile*   Info ()  { return _p_info; }
  bool                Ready () { return _p_info != NULL; }
  void                Update (GLvector2 origin, bool trigger_down);
  SpriteEntry         Hand () { return _p_info->_icon; }
};


#endif // PLAYERWEAPON_H