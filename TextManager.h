#pragma once

#include "common_header.h"
#include "interface.h"
#include "vectors.h"
#include "color.h"

namespace pyrodactyl
{
	//We use this object as the key for all fonts
	typedef unsigned int FontKey;

	class TextManager
	{
		//The place to store all colors
		ColorPool colpool;

		//The rectangle used to store the darkened rectangle coordinates
		SDL_Rect rect;

	public:
		TextManager(){}
		~TextManager(){}

		void Init();
		void Quit(){}
		void Reset(){}

		void Draw(
			const int &x, int y,
			const std::string &text,
			const int &color,
			const FontKey &font,
			const TextAlign &align,
			const int &line_width = INT_MAX,
			const int &line_height = 0u,
			const bool &background = false,
			const bool &use_custom_alpha = false,
			const float &alpha = 1.0f);

		const GLrgba& GetColor(const int &color_id) { return colpool.Get(color_id); }
	};

	extern TextManager gTextManager;
}
