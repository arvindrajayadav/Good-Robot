//=============================================================================
// Author:   Arvind
// Purpose:  Menu class
//=============================================================================
#pragma once

#include "common_header.h"
#include "ImageManager.h"
#include "InputManager.h"
#include "button.h"

namespace pyrodactyl
{
	template <typename T>
	class Menu
	{
	protected:
		//The index of current selected option and highlighted option
		int hover_index;

		//The order in which a keyboard or gamepad traverses the menu
		std::vector<unsigned int> path;

		//Are keyboard buttons enabled?
		bool use_keyboard;

		//Has a key been pressed?
		enum InputDevice { KEYBOARD, MOUSE } latest_input;

		//Do the paths use horizontal, vertical or both types of input for keyboard traversal
		enum PathType { PATH_VERTICAL, PATH_HORIZONTAL, PATH_GRID } path_type;

		//Used for grid type path - the number of button rows
		//The number of elements inside each row is assumed equal
		int rows;

		//Used for path calculations
		int cols, rows_x_cols;

		//------------------------------------------------------------------------
		// Purpose: Find the next element in our path
		//------------------------------------------------------------------------
		void Next()
		{
			if (hover_index < 0 || hover_index >= element.size())
			{
				for (unsigned int pos = 0; pos < path.size(); pos++)
					if (element[path[pos]].Visible())
					{
						hover_index = path[pos];
						break;
					}
			}
			else
			{
				unsigned int curpos = 0;
				for (; curpos < path.size(); curpos++)
					if (path[curpos] == hover_index)
						break;

				for (unsigned int nextloc = (curpos + 1) % element.size(); nextloc != curpos; nextloc = (nextloc + 1) % element.size())
					if (element[nextloc].Visible())
					{
						hover_index = path[nextloc];
						break;
					}
			}
		}

		//------------------------------------------------------------------------
		// Purpose: Find the previous element in our path
		//------------------------------------------------------------------------
		void Prev()
		{
			if (hover_index < 0 || hover_index >= element.size())
			{
				for (unsigned int pos = 0; pos < path.size(); pos++)
					if (element[path[pos]].Visible())
					{
						hover_index = path[pos];
						break;
					}
			}
			else
			{
				unsigned int curpos = 0;
				for (; curpos < path.size(); curpos++)
					if (path[curpos] == hover_index)
						break;

				int nextloc = curpos - 1;
				while (nextloc != curpos)
				{
					if (nextloc < 0)
						nextloc = element.size() - 1;

					if (element[nextloc].Visible())
					{
						hover_index = path[nextloc];
						break;
					}

					nextloc--;
				}
			}
		}

		//------------------------------------------------------------------------
		// Purpose: Traverse a grid style menu using keyboard or joystick
		//------------------------------------------------------------------------
		void MoveGrid(const PageTraverse &dir)
		{
			if (hover_index < 0 || hover_index >= element.size())
			{
				for (unsigned int pos = 0; pos < path.size(); pos++)
					if (element[path[pos]].Visible())
					{
						hover_index = path[pos];
						break;
					}
			}
			else
			{
				//A grid assumes all buttons are visible

				int prev_div = hover_index / rows;
				switch (dir)
				{
				case DIRECTION_DOWN:
					hover_index++;
					if (hover_index / rows != prev_div)
					{
						hover_index -= rows;
						if (hover_index < 0)
							hover_index += element.size();
					}
					else if (hover_index >= element.size())
						hover_index = 0;
					break;
				case DIRECTION_UP:
					hover_index--;
					if (hover_index / rows != prev_div)
					{
						hover_index += rows;
						if (hover_index >= element.size())
							hover_index -= element.size();
					}
					else if (hover_index < 0)
						hover_index = rows - 1;
					break;
				case DIRECTION_RIGHT:
					hover_index += rows;
					if (hover_index >= rows_x_cols)
						hover_index -= rows_x_cols;
					else if (hover_index >= element.size())
					{
						hover_index += rows;
						hover_index -= rows_x_cols;
					}
					break;
				case DIRECTION_LEFT:
					hover_index -= rows;
					if (hover_index < 0)
						hover_index += rows_x_cols;

					if (hover_index >= element.size())
					{
						hover_index -= rows;
					}
					break;
				default: break;
				}
			}
		}

		//------------------------------------------------------------------------
		// Purpose: Handle keyboard input
		//------------------------------------------------------------------------
		int HandleKeyboard()
		{
			if (!element.empty())
			{
				switch (path_type)
				{
				case PATH_VERTICAL:
					if (gInput.Pressed(CONTROL_DOWN))    { Next(); latest_input = KEYBOARD; }
					else if (gInput.Pressed(CONTROL_UP)) { Prev(); latest_input = KEYBOARD; }
					break;
				case PATH_HORIZONTAL:
					if (gInput.Pressed(CONTROL_RIGHT))     { Next(); latest_input = KEYBOARD; }
					else if (gInput.Pressed(CONTROL_LEFT)) { Prev(); latest_input = KEYBOARD; }
					break;
				case PATH_GRID:
					if (gInput.Pressed(CONTROL_DOWN))       { MoveGrid(DIRECTION_DOWN);  latest_input = KEYBOARD; }
					else if (gInput.Pressed(CONTROL_UP))    { MoveGrid(DIRECTION_UP);    latest_input = KEYBOARD; }
					else if (gInput.Pressed(CONTROL_RIGHT)) { MoveGrid(DIRECTION_RIGHT); latest_input = KEYBOARD; }
					else if (gInput.Pressed(CONTROL_LEFT))  { MoveGrid(DIRECTION_LEFT);  latest_input = KEYBOARD; }
					break;
				default: break;
				}

				if (gInput.Pressed(CONTROL_ACTIVATE) && hover_index != -1)
				{
					element.at(hover_index).ToggleRadioState();
					return hover_index;
				}

				//We pressed a key, which means we have to update the hovering status
				if (latest_input == KEYBOARD)
				{
					//Update hover status of keys according to the current index
					unsigned int i = 0;
					for (auto it = element.begin(); it != element.end(); ++it, ++i)
						it->hover_key = (i == hover_index);
				}
			}

			return -1;
		}

	public:
		//The collection of buttons in the menu
		std::vector<T> element;

		Menu() { hover_index = -1; use_keyboard = false; latest_input = MOUSE; path_type = PATH_VERTICAL; rows = 1; cols = 1; rows_x_cols = 1; }
		~Menu(){}

		void Reset()
		{
			latest_input = MOUSE;
			hover_index = -1;
			for (auto b = element.begin(); b != element.end(); ++b)
				b->Reset();
		}

		void SetUI(pyroRect *parent = nullptr)
		{
			for (auto i = element.begin(); i != element.end(); ++i)
				i->SetUI(parent);
		}

		//------------------------------------------------------------------------
		// Purpose: Load the menu from a file
		//------------------------------------------------------------------------
		void Load(rapidxml::xml_node<char> * node)
		{
			if (NodeValid(node))
			{
				LoadBool(use_keyboard, "keyboard", node, false);

				for (auto n = node->first_node(); n != NULL; n = n->next_sibling())
				{
					T b;
					b.Load(n);
					element.push_back(b);
				}

				std::string ty;
				LoadStr(ty, "type", node, false);

				//Only used for grid type menus
				LoadNum(rows, "rows", node);

				if (ty == "grid")   path_type = PATH_GRID;
				else if (ty == "x") path_type = PATH_HORIZONTAL;
				else                path_type = PATH_VERTICAL;

				AssignPaths();
			}
		}

		//------------------------------------------------------------------------
		// Purpose: Event Handling
		// The reason this function doesn't declare its own Event object is because
		// a menu might not be the only object in a game state
		//------------------------------------------------------------------------
		int HandleEvents()
		{
			//The keyboard/joystick event handling bit
			if (use_keyboard)
			{
				int result = HandleKeyboard();

				//We have accepted a menu option using the keyboard
				if (result != -1)
				{
					//Reset the menu state
					Reset();
					return result;
				}
			}

			//Check if we have moved or clicked the mouse
			if (InputKeyState(INPUT_MOUSE_MOVED) || InputKeyState(INPUT_LMB) || InputKeyState(INPUT_RMB))
			{
				//Since the player is moving the mouse, we have to recalculate hover index at every opportunity
				hover_index = -1;
				latest_input = MOUSE;
			}

			//The mouse and hotkey event handling bit
			int i = 0;
			for (auto it = element.begin(); it != element.end(); ++it, ++i)
			{
				//We clicked on a button (or dragged it) using the mouse
				ButtonAction res = it->HandleEvents();
				if (res == BUAC_LCLICK || res == BUAC_GRABBED)
				{
					//Reset the menu state
					Reset();
					return i;
				}

				//We did not click a button, however we did hover over the button
				//However if we are use keyboard to browse through the menu, hovering is forgotten until we move the mouse again
				if (it->hover_mouse && latest_input == MOUSE)
				{
					hover_index = i;

					//The latest input is the mouse, which means we have to forget the keyboard hover states
					for (auto e = element.begin(); e != element.end(); ++e)
						e->hover_key = false;
				}
			}

			if (latest_input == KEYBOARD)
			{
				//The latest input is the keyboard, which means we have to forget the mouse hover states
				for (auto it = element.begin(); it != element.end(); ++it)
					it->hover_mouse = false;
			}

			return -1;
		}

		//------------------------------------------------------------------------
		// Purpose: Draw the menu
		//------------------------------------------------------------------------
		void Draw()
		{
			for (auto it = element.begin(); it != element.end(); ++it)
				it->Draw();
		}

		//------------------------------------------------------------------------
		// Purpose: Get info about the menu
		//------------------------------------------------------------------------
		void UseKeyboard(const bool &val) { use_keyboard = val; }
		void Clear() { element.clear(); }
		const int HoverIndex() { return hover_index; }

		const int Rows() { return rows; }
		void Rows(int val) { rows = val; }

		//------------------------------------------------------------------------
		// Purpose: Assign traversal paths
		//------------------------------------------------------------------------
		void AssignPaths()
		{
			path.clear();
			if (!element.empty())
			{
				//For horizontal and vertical menus, we just navigate them in the order of the buttons
				//For grid menus, this is how vertical navigation works
				for (unsigned int i = 0; i < element.size(); i++)
					path.push_back(i);

				cols = (element.size() / rows) + 1;
				rows_x_cols = rows * cols;
			}
		}
	};

	//A menu with simple buttons
	typedef Menu<Button> ButtonMenu;
}
