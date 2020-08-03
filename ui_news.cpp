#include "master.h"
#include "ui_news.h"

//The news text is stored here
namespace pyrodactyl
{
	static NewsData news_data;

	void LoadNewsText()
	{
		news_data.Load();
	}
}

using namespace pyrodactyl;

void NewsData::Load()
{
	//Store all news lines
	XMLDoc news_doc("core/data/news.xml");
	if (news_doc.ready())
	{
		rapidxml::xml_node<char> *node = news_doc.Doc()->first_node("news");
		if (NodeValid(node))
		{
			for (rapidxml::xml_node<char> *n = node->first_node(); n != nullptr; n = n->next_sibling())
			{
				Level l;
				l.Load(n);
				text.push_back(l);
			}
		}
	}
}

void NewsDisplay::Load(rapidxml::xml_node<char> *node)
{
	ticker.Load(node);

	if (NodeValid("caption", node))
		caption.Load(node->first_node("caption"));
}

void NewsDisplay::Randomize(const int &level)
{
	//No news exists
	if (news_data.text.empty())
		return;

	int level_index = level;
	//If the level doesn't exist in the news, load the first level's news
	if (level >= news_data.text.size())
		level_index = 0;

	//No news in this level
	if (news_data.text.at(level_index).set.empty())
		return;

	//Find out the number of news items
	unsigned size = news_data.text.at(level_index).set.size();

	//Select a random group of news from the level
	int group_index = RandomVal(size);

	//Make sure it's not the same as the last one
	if (group_index == last_text)
		group_index = (group_index + 1) % size;

	last_text = group_index;

	//Set the news
	if (level_index < news_data.text.size())
	{
		for (int i = 0; i < NEWS_SET_SIZE; ++i)
			ModifyTicker(i, news_data.text.at(level_index).set.at(group_index).text[i]);
	}
	ticker.ResetCur();
}

bool NewsDisplay::HandleEvents()
{
	return ticker.HandleEvents();
}

void NewsDisplay::Draw()
{
	caption.Draw();
	ticker.Draw();
}

void NewsDisplay::SetUI()
{
	caption.SetUI();
	ticker.SetUI();
}