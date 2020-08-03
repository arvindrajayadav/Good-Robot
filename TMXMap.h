#pragma once

#include "master.h"
#include "common_header.h"
#include "TMXLayer.h"

namespace pyrodactyl
{
	//For TMX version 1.0, orthogonal maps only
	class TMXMap
	{
	public:
		//Dimensions of the level in terms of tiles
		int tile_rows, tile_cols;

		//The layer of tiles in the level
		MapLayer layer;

		TMXMap();
		~TMXMap(){}

		void Load(const std::string &path, std::string filename);
		void Copy(unsigned char(&map)[PAGE_SIZE][PAGE_SIZE]);
		void CreateDoor(const PageTraverse &dir, unsigned char(&map)[PAGE_SIZE][PAGE_SIZE]);
	};
}