#pragma once

#include "loaders.h"

namespace pyrodactyl
{
	//The data we store in our high score table
	struct HighScoreData
	{
		//Name of the score
		std::string name;

		//Simple integer values
		int upgrade_count, warranty_count;

		//Long values
		long kills, score, money_earned;

		//In milliseconds
		long playtime;

		//This is false by default, but true if the high score data entry is valid
		bool valid;

		HighScoreData() { valid = false; }

		void Load(rapidxml::xml_node<char> *node)
		{
			valid = NodeValid(node);
			if (valid)
			{
				LoadBool(valid, "valid", node);
				LoadStr(name, "name", node);
				LoadNum(score, "val", node);
				LoadNum(kills, "kills", node);
				LoadNum(money_earned, "money", node);
				LoadNum(upgrade_count, "upgrade", node);
				LoadNum(warranty_count, "warranty", node);
				LoadNum(playtime, "time", node);
			}
		}

		void Save(rapidxml::xml_node<char> *root, rapidxml::xml_document<char> &doc)
		{
			SaveBool(valid, "valid", doc, root);
			root->append_attribute(doc.allocate_attribute("name", name.c_str()));
			root->append_attribute(doc.allocate_attribute("val", gStrPool.LGet(score)));
			root->append_attribute(doc.allocate_attribute("kills", gStrPool.LGet(kills)));
			root->append_attribute(doc.allocate_attribute("money", gStrPool.LGet(money_earned)));
			root->append_attribute(doc.allocate_attribute("upgrade", gStrPool.Get(upgrade_count)));
			root->append_attribute(doc.allocate_attribute("warranty", gStrPool.Get(warranty_count)));
			root->append_attribute(doc.allocate_attribute("time", gStrPool.LGet(playtime)));
		}

		bool operator>(const HighScoreData& h) const
		{
			if (score > h.score)
				return true;
			else if (score == h.score)
				return kills > h.kills;

			return false;
		}
	};
}