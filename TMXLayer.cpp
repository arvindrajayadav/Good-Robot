#include "master.h"
#include "TMXLayer.h"
#include "random.h"

using namespace pyrodactyl;

bool Layer::Load(rapidxml::xml_node<char> *node)
{
	if (NodeValid(node))
		return LoadStr(name, "name", node) && LoadNum(w, "width", node) && LoadNum(h, "height", node);

	return false;
}

bool MapLayer::Load(const std::string &path, rapidxml::xml_node<char> *node)
{
	if (Layer::Load(node))
	{
		if (NodeValid("properties", node, false))
		{
			std::string n, v;

			for (auto p = node->first_node("properties")->first_node("property"); p != NULL; p = p->next_sibling("property"))
			{
				if (LoadStr(n, "name", p) && LoadStr(v, "value", p))
				{
					if (n == "x_flip" && v == "false")              allowed_flip[V_XFLIP] = false;
					else if (n == "y_flip" && v == "false")         allowed_flip[V_YFLIP] = false;
					else if (n == "transpose" && v == "false")      allowed_flip[V_TRANSPOSE] = false;
					else if (n == "clockwise" && v == "false")      allowed_flip[V_CLOCKWISE] = false;
					else if (n == "anti-clockwise" && v == "false") allowed_flip[V_ANTICLOCKWISE] = false;
					else if (n == "anti-transpose" && v == "false") allowed_flip[V_ANTITRANSPOSE] = false;
				}
			}
		}

		if (NodeValid("data", node))
		{
			//.tmx stores tiles row-first

			int x = 0, y = 0;
			for (auto n = node->first_node("data")->first_node("tile"); n != nullptr && y < PAGE_SIZE; n = n->next_sibling("tile"))
			{
				tile[x][y].Load(n);

				if (++x >= PAGE_SIZE)
				{
					x = 0;
					++y;
				}
			}

			//We've loaded our layer, now it's time to spin the flipping wheel
			ApplyVariant();

			return true;
		}
	}

	return false;
}

void MapLayer::ApplyVariant()
{
	//Pick a random value out of the ones we are allowed
	std::vector<int> allow;

	for (int i = 0; i < V_TOTAL; ++i)
		if (allowed_flip[i])
			allow.push_back(i);

	int val = RandomVal(allow.size());
	variant = static_cast<LayerVariant>(allow.at(val));

	switch (variant)
	{
	case V_NONE:
		//No changes
		break;

	case V_XFLIP:
		//Flip it along the X axis from the center of the map
		for (int i = 0; i < PAGE_SIZE / 2; ++i) {
			for (int j = 0; j < PAGE_SIZE; j++){
				TileInfo temp = tile[i][j];
				tile[i][j] = tile[PAGE_SIZE - 1 - i][j];
				tile[PAGE_SIZE - 1 - i][j] = temp;
			}
		}
		break;

	case V_YFLIP:
		//Flip it along the Y axis from the center of the map
		for (int i = 0; i < PAGE_SIZE; ++i) {
			for (int j = 0; j < PAGE_SIZE / 2; j++){
				TileInfo temp = tile[i][j];
				tile[i][j] = tile[i][PAGE_SIZE - 1 - j];
				tile[i][PAGE_SIZE - 1 - j] = temp;
			}
		}
		break;

	case V_TRANSPOSE:
		//Transpose the matrix (we know it is a square, which is why we use this method)
		for (int i = 0; i < PAGE_SIZE; ++i) {
			for (int j = 0; j < i; j++){
				TileInfo temp = tile[i][j];
				tile[i][j] = tile[j][i];
				tile[j][i] = temp;
			}
		}
		break;

	case V_CLOCKWISE:
		//Rotate 90 degrees clockwise
		for (int i = 0; i < PAGE_SIZE / 2; i++)
			for (int j = 0; j < (PAGE_SIZE + 1) / 2; j++)
				cyclic_roll_clockwise(tile[i][j], tile[PAGE_SIZE - 1 - j][i], tile[PAGE_SIZE - 1 - i][PAGE_SIZE - 1 - j], tile[j][PAGE_SIZE - 1 - i]);
		break;

	case V_ANTICLOCKWISE:
		//Rotate 90 degrees anticlockwise
		for (int i = 0; i < PAGE_SIZE / 2; i++)
			for (int j = 0; j < (PAGE_SIZE + 1) / 2; j++)
				cyclic_roll_anticlockwise(tile[i][j], tile[PAGE_SIZE - 1 - j][i], tile[PAGE_SIZE - 1 - i][PAGE_SIZE - 1 - j], tile[j][PAGE_SIZE - 1 - i]);
		break;

	case V_ANTITRANSPOSE:
		//This is transposing across the anti-diagonal instead of the diagonal
		//Again, we know levels are square which is why we use this method
		for (int i = 0; i < PAGE_SIZE; ++i) {
			for (int j = 0; j < i; ++j) {
				TileInfo temp = tile[i][j];
				tile[i][j] = tile[PAGE_SIZE - 1 - j][PAGE_SIZE - 1 - i];
				tile[PAGE_SIZE - 1 - j][PAGE_SIZE - 1 - i] = temp;
			}
		}
		break;
	default: break;
	};
}