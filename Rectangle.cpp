#include "master.h"
#include "Rectangle.h"

using namespace pyrodactyl;

bool pyroRect::Load(rapidxml::xml_node<char> *node, const bool &echo, const std::string &x_name, const std::string &y_name,
	const std::string &w_name, const std::string &h_name)
{
	return LoadNum(x, x_name, node, echo)
		&& LoadNum(y, y_name, node, echo)
		&& LoadNum(w, w_name, node, echo)
		&& LoadNum(h, h_name, node, echo);
}

bool pyroRect::Collide(pyroRect box)
{
	if (box.x + box.w < x) return false;	//just checking if their
	if (box.x > x + w)     return false;	//bounding boxes even touch
	if (box.y + box.h < y) return false;
	if (box.y > y + h)	  return false;

	return true; //bounding boxes intersect
}

void pyroRect::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char* name)
{
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);
	child->append_attribute(doc.allocate_attribute("x", gStrPool.Get(x)));
	child->append_attribute(doc.allocate_attribute("y", gStrPool.Get(y)));
	child->append_attribute(doc.allocate_attribute("w", gStrPool.Get(w)));
	child->append_attribute(doc.allocate_attribute("h", gStrPool.Get(h)));
	root->append_node(child);
}