#pragma once

#include "XMLDoc.h"
#include "HoverInfo.h"
#include "random.h"
#include "ValuePicker.h"

#define NEWS_SET_SIZE 3

namespace pyrodactyl
{
	//All possible news
	struct NewsData
	{
		//One set contains 3 lines of text
		struct Set
		{
			//One string of text is a basic unit of news
			std::string text[NEWS_SET_SIZE];

			void Load(rapidxml::xml_node<char> *node)
			{
				int i = 0;
				for (rapidxml::xml_node<char> *n = node->first_node("line"); n != nullptr && i < NEWS_SET_SIZE; n = n->next_sibling("line"), ++i)
					LoadStr(text[i], "text", n);
			}
		};

		//Each level can have multiple zones
		struct Level
		{
			std::vector<Set> set;

			void Load(rapidxml::xml_node<char> *node)
			{
				for (rapidxml::xml_node<char> *n = node->first_node("group"); n != nullptr; n = n->next_sibling("group"))
				{
					Set s;
					s.Load(n);
					set.push_back(s);
				}
			}
		};

		//The list of all the news, sorted according to level
		std::vector<Level> text;

		void Load();
	};

	void LoadNewsText();

	//The news that is displayed on screen
	class NewsDisplay
	{
		//The player can cycle between 3 lines of news text
		struct NewsItem
		{
			int val;
			std::string text;

			void Load(rapidxml::xml_node<char> *node)
			{
				LoadNum(val, "val", node);
			}
		};
		ValuePicker<NewsItem> ticker;

		//The description of the news
		HoverInfo caption;

		//Keep track of the last news in case RNG comes up with the same news
		int last_text;

		void ModifyTicker(const int &pos, const std::string &text)
		{
			NewsItem data;
			data.val = pos;
			data.text = text;

			ticker.ModifyValue(pos, data);
		}

	public:
		NewsDisplay() { last_text = -1; }

		void Load(rapidxml::xml_node<char> *node);

		//To select a news item, we must know the level we are in
		void Randomize(const int &level);

		bool HandleEvents();
		void Draw();
		void SetUI();
	};
}