#include "master.h"
#include "XMLDoc.h"
#include "TextManager.h"

using namespace pyrodactyl;

namespace pyrodactyl
{
	TextManager gTextManager;
}

//------------------------------------------------------------------------
// Purpose: Initialize, set cache etc
//------------------------------------------------------------------------
void TextManager::Init()
{
	colpool.Load("core/data/colors.xml");
}

//------------------------------------------------------------------------
// Purpose: Draw text
//------------------------------------------------------------------------
void TextManager::Draw(
	const int &x, int y,
	const std::string &text,
	const int &color,
	const FontKey &font,
	const TextAlign &align,
	const int &line_width, const int &line_height,
	const bool &background,
	const bool &use_custom_alpha, const float &alpha)
{
	for (int start_pos = 0, len = text.length(); start_pos < len; y += line_height)
	{
		int end_pos = start_pos + 1, last_interrupt = -1;
		std::string word;

		while (end_pos - start_pos <= line_width)
		{
			if (end_pos >= len || text[end_pos] == '`')
			{
				last_interrupt = end_pos;
				break;
			}

			if (text[end_pos] == ' ' || text[end_pos] == ',' || text[end_pos] == '.')
				last_interrupt = end_pos;

			end_pos++;
		}

		if (last_interrupt >= 0) //wrap a word around
		{
			for (int i = 0; i < last_interrupt - start_pos; i++)
				word += text[start_pos + i];

			start_pos = last_interrupt + 1;
		}
		else //word bigger than line, just thunk
		{
			for (int i = 0; i < end_pos - start_pos; i++)
				word += text[start_pos + i];

			start_pos += line_width;
		}

		const Font *f = InterfaceFont(font);
		GLcoord2 c(x, y);
		GLrgba col = colpool.Get(color);
		if (use_custom_alpha)
			col.alpha = alpha;

		int flags = 0;

		if (background) flags = FONTMSG_DROPSHADOW;

		if (align.x == ALIGN_LEFT) flags = flags | FONTMSG_JUSTIFY_LEFT;
		else if (align.x == ALIGN_CENTER) flags = flags | FONTMSG_JUSTIFY_CENTER;
		else flags = flags | FONTMSG_JUSTIFY_RIGHT;

		if (align.y == ALIGN_LEFT) flags = flags | FONTMSG_ALIGN_TOP;
		else if (align.y == ALIGN_CENTER) flags = flags | FONTMSG_ALIGN_CENTER;
		else flags = flags | FONTMSG_ALIGN_BOTTOM;

		f->Print(c, flags, col, word.c_str());
	}
}