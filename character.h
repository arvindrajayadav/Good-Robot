#ifndef CHARACTER_H
#define CHARACTER_H

class Character
{
public:
	string        _name;
	//The color of the body
	GLrgba        _color;
	//The size of each body part.
	float         _size_torso;
	float         _size_head;
	float         _size_eye;
	float         _size_launcher;
	float         _size_laser;
	//Location of part relative to the torso.
	GLvector2     _pos_head;
	GLvector2     _pos_launcher;
	GLvector2     _pos_laser;
	//The sprites from the sprite sheet.
	SpriteEntry   _sprite_torso;
	SpriteEntry   _sprite_head;
	SpriteEntry   _sprite_launcher;
	SpriteEntry   _sprite_laser;
	int           _eye_number;

	float         _speed;

	bool          _ability[ABILITY_TYPES];

private:
	int           AbilityFromName(string name);
public:
	void          Init(class iniFile, string name);
	bool          Ability(int index) { return _ability[index]; }
};

#endif // CHARACTER_H