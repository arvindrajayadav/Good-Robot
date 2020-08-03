#pragma once

#include "master.h"
#include "common_header.h"
#include "TileInfo.h"

namespace pyrodactyl
{
	template<typename T>
	void cyclic_roll_anticlockwise(T &a, T &b, T &c, T &d)
	{
		T temp = a;
		a = b;
		b = c;
		c = d;
		d = temp;
	}

	template<typename T>
	void cyclic_roll_clockwise(T &a, T &b, T &c, T &d)
	{
		T temp = d;
		d = c;
		c = b;
		b = a;
		a = temp;
	}

	class Layer
	{
	public:
		//Name of the layer
		std::string name;

		//Dimensions of the layer in terms of tiles
		int w, h;

		Layer() { w = 0; h = 0; }
		bool Load(rapidxml::xml_node<char> *node);
	};

	//Currently we just use one general purpose layer object instead of multiple inherited classes and stuff
	class MapLayer : public Layer
	{
		enum LayerVariant
		{
			V_NONE,
			V_XFLIP,
			V_YFLIP,
			V_TRANSPOSE,
			V_CLOCKWISE,
			V_ANTICLOCKWISE,
			V_ANTITRANSPOSE,
			V_TOTAL
		};

		//For EXTRA RANDOM FUN, we flip the orientation of the map in one of 6 ways
		//This makes it 7 possible variations of a single map (including the non-flipped version)
		LayerVariant variant;

		//Sometimes pesky "game designers" want to restrict some flip types
		//Find out which ones we're allowed to have
		bool allowed_flip[V_TOTAL];

		void ApplyVariant();

	public:
		//The tiles in the layer
		TileInfo tile[PAGE_SIZE][PAGE_SIZE];

		MapLayer()
		{
			variant = V_NONE;

			//Assume every flip is allowed until told otherwise
			for (int i = 0; i < V_TOTAL; ++i)
				allowed_flip[i] = true;
		}

		bool Load(const std::string &path, rapidxml::xml_node<char> *node);
	};
}