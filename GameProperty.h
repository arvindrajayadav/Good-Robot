#pragma once

#include "common_header.h"
#include "env.h"
#include "loaders.h"

template <typename T>
class GameProperty
{
	//The scaling factor (used in some cases)
	T scaling;

	//Should we use the scaling factor value?
	bool use_scaling;

	//All the levels of this property (size = max)
	std::vector<PropertyData<T>> level;

	//Should we also change the cost based on upgrades purchased?
	//This is multiplied to the level cost and upgrade count
	float upgrade_count_factor;

public:
	GameProperty()
	{
		scaling = 1;
		use_scaling = false;
		upgrade_count_factor = 0.0f;
		level.resize(10);
	}

	//Get property value at a certain level
	PropertyData<T> Get(int index)
	{
		//If our index is a valid number, return the value
		if (index > 0 && index < level.size())
			return level.at(index);

		//If index is invalid and the array isn't empty, return the first value
		if (!level.empty())
			return level.at(0);

		//If index is invalid and the array is empty, something went wrong - return an empty structure
		PropertyData<T> empty;
		return empty;
	}

	//Get the maximum value of this property
	T Max()
	{
		if (!level.empty())
			return level.at(level.size() - 1).val;

		return 0;
	}

	unsigned int Size() { return level.size(); }

	float UpgradeCountFactor() { return upgrade_count_factor; }

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true)
	{
		using namespace pyrodactyl;
		use_scaling = LoadNum(scaling, "scale_factor", node, echo);
		LoadNum(upgrade_count_factor, "upgrade_count_factor", node, echo);

		level.clear();
		for (rapidxml::xml_node<char> *n = node->first_node(); n != nullptr; n = n->next_sibling())
		{
			PropertyData<T> p;
			LoadNum(p.cost, "cost", n, echo);

			LoadNum(p.val, "val", n, echo);
			if (use_scaling)
				p.val *= scaling;

			level.push_back(p);
		}
	}
};
