#pragma once

#include "button.h"

namespace pyrodactyl
{
	//Note: This needs a populate function to work properly
	template <typename T>
	class ValuePicker
	{
		//The previous and next buttons
		Button prev, next;

		//Used for drawing the currently selected value
		TextData text;

		//The currently selected value and the last valid value
		int cur, backup;

		//The list of values
		std::vector<T> val;

	public:
		ValuePicker() { ResetCur(); }

		void Reset() { cur = backup; }
		void ResetCur() { cur = 0; backup = 0; }

		T CurV() { return val[cur]; }
		int CurI() { return cur; }

		void Clear() { val.clear(); }
		void Insert(const T& v) { val.push_back(v); }

		void SetCurrentVal(const T& v)
		{
			for (int i = 0; i < val.size(); ++i)
				if (val[i] == v)
				{
					cur = i;
					backup = i;
					break;
				}
		}

		void ModifyValue(const int &pos, const T &data)
		{
			if (pos < val.size())
				val.at(pos) = data;
		}

		void Load(rapidxml::xml_node<char> *node)
		{
			if (NodeValid("text", node))
			{
				rapidxml::xml_node<char> *tenode = node->first_node("text");
				text.Load(tenode);

				if (NodeValid("prev", tenode))
					prev.Load(tenode->first_node("prev"), &text);

				if (NodeValid("next", tenode))
					next.Load(tenode->first_node("next"), &text);
			}

			if (NodeValid("options", node))
			{
				rapidxml::xml_node<char> *opnode = node->first_node("options");
				for (rapidxml::xml_node<char> *n = opnode->first_node(); n != nullptr; n = n->next_sibling())
				{
					T c;
					c.Load(n);
					Insert(c);
				}
			}
		}

		void Draw()
		{
			if (cur > 0)
				prev.Draw();

			if (cur < val.size() - 1)
				next.Draw();

			//Draw the current value
			text.Draw(val.at(cur).text);
		}

		bool HandleEvents()
		{
			if (cur > 0)
			{
				if (prev.HandleEvents() == BUAC_LCLICK)
				{
					cur--;
					return true;
				}
			}

			if (cur < val.size() - 1)
			{
				if (next.HandleEvents() == BUAC_LCLICK)
				{
					cur++;
					return true;
				}
			}

			return false;
		}

		void SetUI(pyroRect *parent = nullptr)
		{
			text.SetUI(parent);
			prev.SetUI(parent);
			next.SetUI(parent);
		}
	};
}