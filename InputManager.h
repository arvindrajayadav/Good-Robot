#pragma once

#include "common_header.h"
#include "inputval.h"

namespace pyrodactyl
{
	enum InputType
	{
		CONTROL_NONE = -1,

		//These controls can be changed by the player

		CONTROL_UP,
		CONTROL_LEFT,
		CONTROL_DOWN,
		CONTROL_RIGHT,
		CONTROL_ACTIVATE,
		CONTROL_PREV,
		CONTROL_NEXT,

		//These controls cannot be changed by the player

		CONTROL_BACK,
		CONTROL_PAUSE,
		CONTROL_PRIMARY,
		CONTROL_SECONDARY,
		CONTROL_LEADERBOARD_PREV,
		CONTROL_LEADERBOARD_NEXT
	};

	const int CONTROL_REBINDABLE_COUNT = CONTROL_NEXT + 1, CONTROL_COUNT = CONTROL_LEADERBOARD_NEXT + 1;

	class InputManager
	{
		//The backups used to restore in case of the user pressing cancel
		InputVal backup[CONTROL_COUNT];

		//Load key configuration from file
		void Load(const std::string &filename);

		//The current version of the input scheme
		unsigned int version;

		//The textures for the various hotkey images (located in the big sprite sheet)
		struct
		{
			std::string a, b, x, y, lb, rb, lt, rt, ls, rs, back, start, down, left, right, up;
		} hotkey;

	public:
		InputManager() { version = 0; }
		~InputManager() {}
		void Quit() {}

		//NOTE: The lower level arrays can have buttons in common, but buttons cannot be common within these arrays
		//Ex. UI and Fight can have buttons in common, but not two keys within UI

		//Inputs used in the game
		InputVal iv[CONTROL_COUNT];

		//These functions check if a key has been pressed and then released
		const bool Pressed(const InputType &val);

		//These functions return true if key is held down, false otherwise
		const bool State(const InputType &val);

		//These functions return position of joystick, range of +/- 1.0f
		const float Delta(const InputType &val);

		//Used for "use right analog stick to fire"
		bool RightAxisFire();

		void CreateBackup();
		void RestoreBackup();

		//Initialize the input system
		void Init();

		void Save();

		//Draw a hotkey
		void Draw(const InputType &type, const int &x, const int &y, const int &w, const int &h);
	};

	//The saved key values
	extern InputManager gInput;
}