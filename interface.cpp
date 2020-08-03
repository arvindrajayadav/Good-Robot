/*-----------------------------------------------------------------------------

  Interface.cpp

  Interface manager.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "font.h"
#include "hud.h"
#include "interface.h"
#include "render.h"

#include "loaders.h"
#include "XMLDoc.h"

static vector<string>   print;
static vector<Font*>    font;
static GLcoord2         matte_size;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void InterfacePrint(const char* message, ...)
{
	va_list           marker;
	char              msg_text[1024];

	va_start(marker, message);
	vsprintf(msg_text, message, marker);
	va_end(marker);
	print.push_back(msg_text);
  matte_size.x = max (matte_size.x, (int)strlen (msg_text));
  matte_size.y = print.size ();
}

void InterfaceRender2D()
{
	GLcoord2      size;
	const Font*   f;
	GLcoord2      pos;

	f = InterfaceFont(FONT_FIXEDWIDTH);
	size = RenderViewportSize();
  pos.y = size.y - print.size () * f->Height ();
  glColor4f (0.06f, 0, 0.12f, 0.75f);
  matte_size *= f->Height ();
  glDisable (GL_TEXTURE_2D);
  glBegin (GL_QUADS);
  glVertex2d (0, pos.y);
  glVertex2d (matte_size.x, pos.y);
  glVertex2d (matte_size.x, size.y);
  glVertex2d (0, size.y);
  glEnd ();
  matte_size = GLcoord2 ();
  glColor3f(1, 1, 1);
  //pos.y = f->Height ();
	for (unsigned i = 0; i < print.size(); i++) {
		f->Print(pos, print[i].c_str());
		pos.y += f->Height();
	}
	print.clear();
  glEnable (GL_TEXTURE_2D);
}

void InterfaceInit()
{
	using namespace pyrodactyl;

	XMLDoc font_list("core/fonts/fonts.xml");
	if (font_list.ready()) {
		rapidxml::xml_node<char> *node = font_list.Doc()->first_node("fonts");

		for (auto n = node->first_node("font"); n != NULL; n = n->next_sibling("font"))	{
			rapidxml::xml_attribute<char> *id, *path, *size;
			id = n->first_attribute("id");
			path = n->first_attribute("path");
			size = n->first_attribute("size");

			if (id != NULL && path != NULL && size != NULL)	{
				unsigned int pos = StringToNumber<unsigned int>(id->value());
				if (font.size() <= pos)
					font.resize(pos + 1);
				font.at(pos) = FontCreate(path->value(), StringToNumber<int>(size->value()));
			}
		}
	}
}

const Font* InterfaceFont(const unsigned int id)
{
	return font[id];
}