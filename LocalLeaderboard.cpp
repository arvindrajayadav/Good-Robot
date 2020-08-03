#include "master.h"
#include "LocalLeaderboard.h"
#include "system.h"
#include "file.h"

using namespace pyrodactyl;

//-----------------------------------------------------------------------------
// Purpose: Write score to file if it's in the top max_entries
//-----------------------------------------------------------------------------
void LocalLeaderboard::AddScore(const HighScoreData &h)
{
	//Go through our list of scores to find the appropriate place for this score
	//Our list is sorted in descending order, i.e. largest score at 0, smallest score at max_entries - 1
	int i = 0;
	for (; i < max_entries && i < score.size(); ++i)
	{
		//We also allow a score if the list has an empty space in it
		if (h > score.at(i) || !score.at(i).valid)
			break;
	}

	//This score is good enough to be on the list
	if (i < max_entries)
	{
		score.insert(score.begin() + i, h);

		//If our list is too big, remove the last item(s) from the list
		if (score.size() > max_entries)
			score.resize(max_entries);

		for (int i = 0; i < score.size() && i < max_entries; ++i)
			menu.element.at(i).SetText(score.at(i));

		menu.AssignPaths();
		Save();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Load interface layout and score values from file
//-----------------------------------------------------------------------------
void LocalLeaderboard::Load(rapidxml::xml_node<char> *node)
{
	//First load the interface
	LoadNum(max_entries, "max_entries", node);

	if (NodeValid("legend", node))
	{
		legend.Load(node->first_node("legend"));
		legend.Valid(true);
	}

	if (NodeValid("ref", node))
		ref.Load(node->first_node("ref"));

	if (NodeValid("inc", node))
		inc.Load(node->first_node("inc"));

	//This only loads the menu arrangement and not the contents
	if (NodeValid("menu", node))
		menu.Load(node->first_node("menu"));

	//Initialize the menu
	menu.Clear();
	for (int i = 0; i < max_entries; ++i)
	{
		ScoreButton sb;
		sb.Init(ref, nullptr, inc.RawPosX() * i, inc.RawPosY() * i);
		menu.element.push_back(sb);
	}
	menu.AssignPaths();

	//End of loading interface

	//Now, load up scores from file
	XMLDoc doc(SystemSavePath() + "score.xml");
	if (doc.ready())
	{
		rapidxml::xml_node<char> *node = doc.Doc()->first_node("scores");
		if (NodeValid(node))
		{
			int i = 0;
			for (rapidxml::xml_node<char> *n = node->first_node(); n != nullptr && i < max_entries && i < menu.element.size(); n = n->next_sibling(), ++i)
			{
				HighScoreData h;
				h.Load(n);
				score.push_back(h);

				menu.element.at(i).SetText(h);
			}
		}
	}
	else
	{
		//We can't find the high scores doc, fill our list with empty scores
		for (int i = 0; i < max_entries; ++i)
		{
			HighScoreData h;
			menu.element.at(i).SetText(h);
		}
	}
}

void LocalLeaderboard::Save()
{
	rapidxml::xml_document<char> doc;

	//XML declaration
	{
		rapidxml::xml_node<char> *decl = doc.allocate_node(rapidxml::node_declaration);
		decl->append_attribute(doc.allocate_attribute("version", "1.0"));
		decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
		doc.append_node(decl);
	}

	//Root node
	rapidxml::xml_node<char> *root = doc.allocate_node(rapidxml::node_element, "scores");

	//Push all scores inside root node
	for (unsigned int i = 0; i < score.size(); ++i)
	{
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "s");
		score.at(i).Save(child, doc);
		root->append_node(child);
	}

	doc.append_node(root);

	//Put all this to a string
	std::string xml_as_string;
	rapidxml::print(std::back_inserter(xml_as_string), doc);

	//Save to file, then delete the XML data
	FileSave(SystemSavePath() + "score.xml", xml_as_string.c_str(), xml_as_string.length());
	doc.clear();
}

void LocalLeaderboard::HandleEvents()
{
	menu.HandleEvents();
}

void LocalLeaderboard::Draw()
{
	legend.Draw();
	menu.Draw();
}

void LocalLeaderboard::SetUI()
{
	ref.SetUI();
	inc.SetUI();

	legend.SetUI();
	menu.SetUI();
}