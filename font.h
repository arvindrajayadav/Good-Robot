#ifndef FONT_H
#define FONT_H

#define MAX_CHARS     256
#define AUTO_CHARS    128

#define FONTMSG_JUSTIFY_LEFT    0
#define FONTMSG_JUSTIFY_RIGHT   1
#define FONTMSG_JUSTIFY_CENTER  2

#define FONTMSG_ALIGN_TOP       0
#define FONTMSG_ALIGN_BOTTOM    4
#define FONTMSG_ALIGN_CENTER    8

#define FONTMSG_DROPSHADOW      16



struct FontChar
{
	FontChar(uchar ch, GLrgba c) : color(c), ascii(ch) {}
	FontChar(uchar ch) : color(GLrgba(1, 1, 1)), ascii(ch) {}
	FontChar() : color(GLrgba()), ascii(0) {}

	GLrgba      color;
	uchar       ascii;
};

class Font
{
	string            _filename;              //Filename of the TT font itself.
	unsigned          _rows;			            //The number of rows that can fit on the screen.
	unsigned          _height;	              //Height in pixels.
	unsigned          _textures[MAX_CHARS];	  //Holds the texture id's
	unsigned          _width[MAX_CHARS];      //Width of each character.
	GLuvFrame         _uv[MAX_CHARS];         //The uv rectangles of ech character.
	int               _list_base;	            //Holds the first display list id

	void              BuildList(void* face, unsigned char ch);
public:
	void              AddChar(uchar id, int width, unsigned texture, GLuvFrame frame);
	unsigned          Height() const { return _height; }
	void              Init(const char * fname, unsigned int rows_per_screen);
	void              Generate();
	void              Clean();

	vector<FontChar>  Parse(const char* cstring, GLrgba default_color = GLrgba(1, 1, 1)) const;
	int               Print(GLcoord2 pos, const char* msg) const;
  int               Print(GLcoord2 pos, unsigned flags, GLrgba color, const char* msg) const;
	int               Print(GLcoord2 pos, vector<FontChar> f) const;
	GLcoord2          PushScreen() const;
	void              PopScreen() const;
	unsigned          Width(const char* msg) const;
	unsigned          Width(vector<FontChar> f) const;
	bool              Valid();
};

Font*   FontCreate(char* filename, int rows_per_screen);
void    FontInit();
void    FontRender();
void		FontResize ();
void    FontUpdate();
void    FontValidate();

#endif // FONT_H