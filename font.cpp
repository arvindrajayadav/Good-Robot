/*-----------------------------------------------------------------------------

  Font.cpp

  This handles our interface with freetype.

  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_GLYPH_H
#include FT_TRIGONOMETRY_H

#include "bodyparts.h"
#include "font.h"

static FT_Library     library;
static vector<Font*>  font_list;
static bool           validate_needed;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

GLcoord2 Font::PushScreen() const
{
	GLcoord2    size;
	GLint	viewport[4];

	glPushAttrib(GL_TRANSFORM_BIT);
	glGetIntegerv(GL_VIEWPORT, viewport);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(viewport[0], viewport[2], viewport[3], viewport[1]);
	glPopAttrib();
	size.x = viewport[2] - viewport[0];
	size.y = viewport[3] - viewport[1];
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	return size;
}

void Font::PopScreen() const
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
}

vector<FontChar> Font::Parse(const char* cstring, GLrgba default_color) const
{
	vector<FontChar>    result;
	FontChar            current;
	int                 n;
	int                 tag_size;
	const char*         tag_end;
	char                tag[20];

	current.color = default_color;
	n = 0;
	while (cstring[n]) {
		memset(tag, 0, sizeof(tag));
		if (cstring[n] == '{') {
			tag_end = strchr(cstring + n + 1, '}');
			if (!tag_end) {//no closing marker. Abort the rest of string.
				break;
			}	else { //there's an ending marker
				tag_size = tag_end - (cstring + n);
				strncpy(tag, cstring + n + 1, tag_size);
				if (tag[0] == '@') {
					current.ascii = (uchar)atoi(tag + 1);
					result.push_back(current);
				}	else if (!strncmp(tag, "###", 3)) //use default color
					current.color = default_color;
				else if (tag[0] == '#')
					current.color = GLrgbaFromHex(tag + 1);
				else if (tag[0] == '/') {
					char* c = &tag[0];

					while (c[0] == '/') {
						current.ascii = 10;
						result.push_back(current);
						c++;
					}
				}
				n += tag_size;
			}
		}	else { //just push the character onto the queue
			current.ascii = cstring[n];
			result.push_back(current);
		}
		n++;
	}
	return result;
}

void Font::Init(const char * fname, unsigned int rows_per_screen)
{
	_rows = rows_per_screen;
	_filename = fname;
	_list_base = 0;
}

void Font::Generate()
{
	FT_Library  library;
	FT_Face     face;
	GLint	      viewport[4];
	int         screen_height;

	//Look at the screen size and the number of rows to figure out how
	//big the font needs to be,
	glGetIntegerv(GL_VIEWPORT, viewport);
	screen_height = viewport[3] - viewport[1];
	_height = screen_height / _rows;
	//Initialize everything.
	memset(&_width, 0, sizeof(_width));
	memset(&_textures, 0, sizeof(_textures));
	//Create and initilize a freetype font library.
	if (FT_Init_FreeType(&library)) {
		Console("FT_Init_FreeType failed");
		return;
	}
	//FT_New_Face will die if the font file does not exist or is somehow broken.
	if (FT_New_Face(library, _filename.c_str(), 0, &face)) {
		Console("FT_New_Face failed to load '%s'", _filename.c_str());
		return;
	}
	//For some twisted reason, Freetype measures font size
	//in terms of 1/64ths of pixels.
	FT_Set_Char_Size(face, _height * 64, _height * 48, 96, 96);
	//Allocate id's for all the textures and displays lists we're about to create.
	if (!_list_base)
		_list_base = glGenLists(MAX_CHARS);
	glGenTextures(AUTO_CHARS, _textures);
	//Build 'em.
	for (int i = 0; i < AUTO_CHARS; i++)
		BuildList(face, (uchar)i);
	for (int i = AUTO_CHARS; i < MAX_CHARS; i++)
		BuildList(face, 32);
	//We don't need freetype anymore.
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}

void Font::AddChar(uchar id, int width, unsigned texture, GLuvFrame frame)
{
	_width[id] = width;
	_textures[id] = texture;
	_uv[id] = frame;
	glNewList(_list_base + id, GL_COMPILE);
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, _textures[id]);
	glBegin(GL_QUADS);
	glTexCoord2fv(&_uv[id].uv[0].x);  glVertex2i(0, 0);
	glTexCoord2fv(&_uv[id].uv[1].x);  glVertex2i(width, 0);
	glTexCoord2fv(&_uv[id].uv[2].x);  glVertex2i(width, _height);
	glTexCoord2fv(&_uv[id].uv[3].x);  glVertex2i(0, _height);
	glEnd();
	glPopMatrix();
	glTranslated(_width[id], 0, 0);
	glEndList();
}

//Create a display list coresponding to the given character.
void Font::BuildList(void* face_in, unsigned char ch)
{
	FT_Glyph        glyph;
	FT_Bitmap*      bitmap;
	GLcoord2        bitsize;
	GLcoord2        glyph_offset;
	int             index;
	unsigned char*  expanded_data;
	GLvector2       uv;
	FT_Face         face = (FT_Face)face_in;

	//Load the Glyph for our character.
	if (FT_Load_Glyph(face, FT_Get_Char_Index(face, ch), FT_LOAD_DEFAULT))
		Console("FT_Load_Glyph failed on '%c'", ch);
	if (FT_Get_Glyph(face->glyph, &glyph))
		Console("FT_Get_Glyph failed on '%c'", ch);

	//Convert the glyph to a bitmap.
	FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
	FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
	bitmap = &bitmap_glyph->bitmap;
	//Create a power-of-two buffer for the data
	bitsize.x = PowerOf2(bitmap->width);
	bitsize.y = PowerOf2(bitmap->rows);
	expanded_data = new GLubyte[2 * bitsize.x*bitsize.y];

	//Transfer the font bitmap into the power-of-2 sized buffer.
	for (int j = 0; j < bitsize.y; j++) {
		for (int i = 0; i < bitsize.x; i++){
			index = 2 * (i + j*bitsize.x);
			expanded_data[index] = 255;
			if (i >= bitmap->width || j >= bitmap->rows)
				expanded_data[index + 1] = 0;
			else
				expanded_data[index + 1] = (bitmap->buffer[i + bitmap->width*j]);
		}
	}
	//Create the GL texture.
	glBindTexture(GL_TEXTURE_2D, _textures[ch]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitsize.x, bitsize.y,
		0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);
	delete[] expanded_data;

	//So now we can create the display list
	glNewList(_list_base + ch, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, _textures[ch]);
	glPushMatrix();
	//Ajust the postion of the polygon to allow for proper character spacing.
	glyph_offset.x = bitmap_glyph->left;
	//Convert from bottom-up to top-down coord system.
	glyph_offset.y = _height - bitmap_glyph->bitmap.rows;
	//This is important for "dropped" letters like g and y, which sit below the line.
	glyph_offset.y -= bitmap_glyph->top - bitmap->rows;
	//Now move to the calculated position and draw the polygon.
	glTranslated(glyph_offset.x, glyph_offset.y, 0);
	//Our texture was padded out to the nearest power of 2. Adjust the UV's
	//so we're only referencing the image and not the PO2 padding.
	uv.x = (float)bitmap->width / (float)bitsize.x;
	uv.y = (float)bitmap->rows / (float)bitsize.y;
	_uv[ch].uv[0] = GLvector2(0, 0);
	_uv[ch].uv[1] = GLvector2(uv.x, 0);
	_uv[ch].uv[2] = GLvector2(uv.x, uv.y);
	_uv[ch].uv[3] = GLvector2(0, uv.y);
	glBegin(GL_QUADS);
	glTexCoord2fv(&_uv[ch].uv[0].x);  glVertex2i(0, 0);
	glTexCoord2fv(&_uv[ch].uv[1].x);  glVertex2i(bitmap->width, 0);
	glTexCoord2fv(&_uv[ch].uv[2].x);  glVertex2i(bitmap->width, bitmap->rows);
	glTexCoord2fv(&_uv[ch].uv[3].x);  glVertex2i(0, bitmap->rows);
	glEnd();
	glPopMatrix();
	//Now move to the end of the character and store the width for future
	//formatting calculations.
	_width[ch] = face->glyph->advance.x / 64;
	glTranslated(_width[ch], 0, 0);
	glEndList();
}

int Font::Print(GLcoord2 pos, const char* msg) const
{
	float   modelview_matrix[16];
	int     prev_texture;

	//SAVE ALL THE STATES.
	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glListBase(_list_base);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_texture);
	//Begin drawing.
	glPushMatrix();
	//glLoadIdentity();
	glTranslated(pos.x, pos.y, 0);
	//glMultMatrixf(modelview_matrix);
	glCallLists(strlen(msg), GL_UNSIGNED_BYTE, msg);
	//RESTORE ALL THE STATES
	glPopMatrix();
	glPopAttrib();
	glBindTexture(GL_TEXTURE_2D, prev_texture);
	return Width(msg);
}

int Font::Print(GLcoord2 pos, unsigned flags, GLrgba color, const char* msg) const
{
  GLcoord2  size;
  GLcoord2  origin;

  size.x = Width (msg);
  size.y = _height;
  origin = pos;
  //Horizontal positioning.
  if (flags & FONTMSG_JUSTIFY_RIGHT) 
    origin.x -= size.x;
  if (flags & FONTMSG_JUSTIFY_CENTER)
    origin.x -= size.x/2;
  if (flags & FONTMSG_ALIGN_TOP)
    origin.y -= size.y;
  if (flags & FONTMSG_ALIGN_CENTER)
    origin.y -= size.y / 2;
  if (flags & FONTMSG_DROPSHADOW) {
    glColor3f (0, 0, 0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Print(origin + GLcoord2(2, 2), msg);
  }
  glColor3fv (&color.red);
  return Print(origin, msg);
}


int Font::Print(GLcoord2 pos, vector<FontChar> f) const
{
	float   modelview_matrix[16];
	int     prev_texture;
	int     width;

	//SAVE ALL THE STATES.
	glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
	glMatrixMode(GL_MODELVIEW);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glListBase(_list_base);
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_texture);
	//Begin drawing.
	glPushMatrix();
	//glLoadIdentity();
	glTranslated(pos.x, pos.y, 0);
	//glMultMatrixf(modelview_matrix);
	width = 0;
	for (unsigned i = 0; i < f.size(); i++) {
		glColor3fv(&f[i].color.red);
		width += _width[f[i].ascii];
		glCallLists(1, GL_UNSIGNED_BYTE, &f[i].ascii);
	}
	//RESTORE ALL THE STATES
	glTranslated(-pos.x, -pos.y, 0);
	glPopMatrix();
	glPopAttrib();
	glBindTexture(GL_TEXTURE_2D, prev_texture);
	return width;
}

unsigned Font::Width(const char* msg) const
{
	unsigned      result;
	uchar         index;

	result = 0;
	for (unsigned i = 0; i < strlen(msg); i++) {
		index = msg[i];
		result += _width[index];
	}
	return result;
}

unsigned Font::Width(vector<FontChar> f) const
{
	unsigned      result;
	uchar         index;

	result = 0;
	for (unsigned i = 0; i < f.size(); i++) {
		index = f[i].ascii;
		result += _width[index];
	}
	return result;
}

bool Font::Valid()
{
	if (glIsTexture(_textures[33]))
		return true;
	return false;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void font_init(Font* f)
{
	f->Generate();
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void FontInit()
{
	if (FT_Init_FreeType(&library)) {
		Console("FontInit: Failed to start freetype! I have no idea how anyone will read this error.");
		return;
	}
	Console("FontInit: FreeType Ok.");
}

void FontValidate()
{
	validate_needed = true;
}

Font* FontCreate(char* filename, int rows_per_screen)
{
	Font*   f;

	f = new Font;
	font_list.push_back(f);
	f->Init(filename, rows_per_screen);
	font_init(f);
	f->Generate();
	return f;
}

void FontResize ()
{
	Console ("FontUpdate: Reloading %d fonts.", font_list.size ());
	for (unsigned i = 0; i < font_list.size (); i++)
		font_init (font_list[i]);
}

void FontUpdate()
{
	if (!validate_needed)
		return;
	validate_needed = false;
	if (font_list.empty())
		return;
	if (font_list[0]->Valid())
		return;
	Console("FontUpdate: Reloading %d fonts.", font_list.size());
	for (unsigned i = 0; i < font_list.size(); i++)
		font_init(font_list[i]);
}

void FontRender()
{
}