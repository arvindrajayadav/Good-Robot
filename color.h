#pragma once

#include "common_header.h"
#include "XMLDoc.h"
#include "loaders.h"

namespace pyrodactyl
{
	//Storage pool for saving colors from one file, then using them
	class ColorPool
	{
		std::vector<GLrgba> pool;

		//Default invalid color
		GLrgba invalid;

	public:
		ColorPool() { pool.clear(); invalid.red = 1.0f; invalid.green = 0.0f; invalid.blue = 220.0f; invalid.alpha = 200.0f; }

		const GLrgba& Get(const int &num);
		void Load(const std::string &filename);
	};
}