#include "master.h"
#include "inputval.h"

using namespace pyrodactyl;

bool InputAxisData::Pressed()
{
	if (id > -1)
	{
		if (!toggle)
			toggle = (greater && InputAxis(id) > val) || (!greater && InputAxis(id) < val);
		else
		{
			if (!((greater && InputAxis(id) > val) || (!greater && InputAxis(id) < val)))
			{
				toggle = false;
				return true;
			};
		}
	}

	return false;
}

bool InputAxisData::State()
{
	if (id > -1)
		return (greater && InputAxis(id) > val) || (!greater && InputAxis(id) < val);

	return false;
}

void InputAxisData::LoadState(rapidxml::xml_node<char> *node)
{
	LoadEnum(id, "id", node);
	LoadNum(val, "val", node);
	LoadBool(greater, "greater", node);
}

void InputAxisData::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
{
	rapidxml::xml_node<char> *child;
	child = doc.allocate_node(rapidxml::node_element, "axis");

	child->append_attribute(doc.allocate_attribute("id", gStrPool.Get(id)));
	child->append_attribute(doc.allocate_attribute("val", gStrPool.Get(val)));

	SaveBool(greater, "greater", doc, child);

	root->append_node(child);
}