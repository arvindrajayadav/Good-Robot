#pragma once

#include "common_header.h"
#include "XMLDoc.h"
#include "loaders.h"

namespace pyrodactyl
{
	//Storage pool used for saving numbers as strings
	//We need this because rapidXML stores everything as pointers and they need
	//to remain in memory until the file has been written to disk
	class StringPool
	{
		//Store integer strings here
		std::unordered_map<int, std::string> pool_i;

		//Store long strings here
		std::unordered_map<long, std::string> pool_l;

		//Store floating point strings here
		struct FloatString
		{
			float val;
			std::string str;

			FloatString() { val = 0.0f; }
		};

		std::list<FloatString> pool_f;

	public:
		StringPool() { pool_i.clear(); pool_f.clear(); }

		const char* Get(const int &num)
		{
			if (pool_i.count(num) == 0)
				pool_i[num] = NumberToString(num);

			return pool_i.at(num).c_str();
		}

		const char* LGet(const long &num)
		{
			if (pool_l.count(num) == 0)
				pool_l[num] = NumberToString(num);

			return pool_l.at(num).c_str();
		}

		const char* FGet(const float &num)
		{
			for (auto &i : pool_f)
				if (i.val == num)
					return i.str.c_str();

			FloatString fs;
			fs.val = num;
			fs.str = NumberToString<float>(num);
			pool_f.push_back(fs);

			auto ret = pool_f.rbegin();
			return ret->str.c_str();
		}
	};

	//Strings are stored here to avoid duplicates and invalid values when writing to XML
	extern StringPool gStrPool;
}