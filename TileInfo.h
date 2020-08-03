#pragma once

#include "common_header.h"
#include "loaders.h"

namespace pyrodactyl
{
	enum TextureFlipType
	{
		//Draw texture normally
		FLIP_NONE,

		//Flipped horizontally
		FLIP_X,

		//Flipped vertically
		FLIP_Y,

		//Flipped horizontally and vertically
		FLIP_XY,

		//Flipped anti-diagonally, where anti-diagonal is from top right corner to bottom left
		FLIP_D,

		//Flipped both anti-diagonally and horizontally
		FLIP_DX,

		//Flipped both anti-diagonally and vertically
		FLIP_DY,

		//Flipped anti-diagonally and horizontally and vertically - MAXIMUM FLIP
		FLIP_XYD
	};

	typedef unsigned int GidFormat;

	// Bits on the far end of the 32-bit global tile ID are used for tile flags
	const static GidFormat FlippedHorizontallyFlag = 0x80000000;
	const static GidFormat FlippedVerticallyFlag = 0x40000000;
	const static GidFormat FlippedAntiDiagonallyFlag = 0x20000000;

	struct TileInfo
	{
		//The gid of the tile
		GidFormat gid;

		//Do we need to flip this tile?
		TextureFlipType flip;

		TileInfo() { gid = 0; flip = FLIP_NONE; }
		TileInfo(rapidxml::xml_node<char> *node) { Load(node); }

		void Load(rapidxml::xml_node<char> *node)
		{
			//Load the gid of the tile
			if (!LoadNum(gid, "gid", node))
				gid = 0;

			bool horizontal = (gid & FlippedHorizontallyFlag) != 0;
			bool vertical = (gid & FlippedVerticallyFlag) != 0;
			bool antidiagonal = (gid & FlippedAntiDiagonallyFlag) != 0;

			//Find how the tile is flipped
			if (horizontal)
			{
				if (vertical)
				{
					if (antidiagonal)
						flip = FLIP_XYD;
					else
						flip = FLIP_XY;
				}
				else if (antidiagonal)
					flip = FLIP_DX;
				else
					flip = FLIP_X;
			}
			else if (vertical)
			{
				if (antidiagonal)
					flip = FLIP_DY;
				else
					flip = FLIP_Y;
			}
			else if (antidiagonal)
				flip = FLIP_D;
			else
				flip = FLIP_NONE;

			// Clear the flags
			gid &= ~(FlippedHorizontallyFlag | FlippedVerticallyFlag | FlippedAntiDiagonallyFlag);
		}
	};
}