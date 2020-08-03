/*-----------------------------------------------------------------------------

  Console.cpp

  Drop-down "quake-style" console.

  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"
#include <stdarg.h>

#include "font.h"
#include "game.h"
#include "interface.h"
#include "render.h"
#include "system.h"

#define MAX_MSG_LEN     2048  // 2k
#define LINE_SIZE       80

struct Character
{
	GLrgba        color;
	char          ascii;
};

struct Line
{
	vector<FontChar>    characters;
	void                Clear()  { characters.clear(); }
};

static const GLrgba   color_console = GLrgba(0.33f, 0.33f, 1.0f, 0.5f);
static const GLrgba   color_border = GLrgba(1, 1, 1);
static const GLrgba   color_input = GLrgba(0, 1, 1);
static const GLrgba   color_notify = GLrgba(1, 1, 0);
static vector<string> queue;
static vector<string> history;
static vector<Line>   lines;
static bool           ready;
static bool           is_open;
static Line           input;
static int            history_pos;
static int            current_line;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void restore_history()
{
	const char* buffer;
	FontChar    fc;

	if (history.size() == 0)
		return;
	buffer = history[history_pos].c_str();
	input.Clear();
	for (unsigned i = 0; i < strlen(buffer); i++) {
		fc = FontChar(buffer[i], color_input);
		input.characters.push_back(fc);
	}
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void ConsoleCommand(Line l)
{
	string  cmd;
	char    tmp[2];

	lines.push_back(l);
	for (unsigned i = 0; i < l.characters.size(); i++) {
		tmp[0] = l.characters[i].ascii;
		tmp[1] = 0;
		cmd.append(tmp);
	}
	history.push_back(cmd.c_str());
	history_pos = history.size() - 1;
	GameCommand(cmd.c_str());
}

void ConsoleInit()
{
	history_pos = -1;
}

void ConsoleToggle()
{
	is_open = !is_open;
}

void ConsoleInput(int key, int char_code)
{
	FontChar      fc;

	if (key == SDLK_UP) {
		restore_history();
		if (history_pos > 0)
			history_pos--;
	}
	if (key == SDLK_DOWN) {
		restore_history();
		if (history_pos < (int)history.size() - 1)
			history_pos++;
	}
	if (char_code >= 32) { //typing normal characters to the input line.
		if (char_code > 255)
			return;
		fc = FontChar((uchar)char_code, color_input);
		input.characters.push_back(fc);
	}
	if (key == SDLK_RETURN && !input.characters.empty()) {
		ConsoleCommand(input);
		input.Clear();
	}
	if (key == SDLK_BACKSPACE && !input.characters.empty())
		input.characters.pop_back();
}

bool ConsoleIsOpen()
{
	return is_open;
}

void ConsoleRender()
{
	GLrgba      color;
	GLcoord2    size;
	GLcoord2    pos;
	int         bottom;
	const Font* font;
	int         display_lines;

	if (!is_open)
		return;

	font = InterfaceFont(FONT_FIXEDWIDTH);
	size = font->PushScreen();
	display_lines = (size.y / font->Height()) / 2;
	display_lines += display_lines / 2;
	//Bottom edge of console window.
	bottom = (display_lines + 1) * font->Height();
	//Leave a bit of a margin.
	bottom += font->Height() / 4;
	//Draw the console background.
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(false);
	glColor4fv(&color_console.red);
	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(size.x, 0);
	glVertex2i(size.x, bottom);
	glVertex2i(0, bottom);
	glEnd();
	glColor4fv(&color_border.red);
	glBegin(GL_LINES);
	glVertex2i(0, bottom);
	glVertex2i(size.x, bottom);
	glEnd();
	//Draw the contents of the console.
	pos = GLcoord2();
	for (int i = 0; i < display_lines; i++) {
		int index;

		index = (lines.size() - display_lines) + i;
		if (index >= 0) {
			font->Print(pos, lines[index].characters);
		}
		pos.y += font->Height();
	}
	//Draw the user input line
	pos.x += font->Print(pos, ">");
	pos.x += font->Print(pos, input.characters);
	font->Print(pos, "_");
	font->PopScreen();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
}

void ConsoleUpdate()
{
	GLrgba      color;
	unsigned    i;
	const char* c_str;
	FontChar    fc;
	Line        newline;
	int         colon;
	int         last_space;

	color = GLrgba(1, 1, 1);
	for (i = 0; i < queue.size(); i++) {
		c_str = queue[i].c_str();
		newline.Clear();
		last_space = 0;
		for (unsigned j = 0; j < strlen(c_str); j++) {
			fc = FontChar(c_str[j], color);
			newline.characters.push_back(fc);
			if (newline.characters.size() > 80) {
				vector<FontChar>  endline;

				endline.insert(endline.begin(), newline.characters.begin() + last_space, newline.characters.end());
				newline.characters.erase(newline.characters.begin() + last_space, newline.characters.end());
				lines.push_back(newline);
				newline.Clear();
				newline.characters.insert(newline.characters.begin(), endline.begin(), endline.end());
				last_space = 0;
			}
			if (c_str[j] == 13) {
				lines.push_back(newline);
				newline.Clear();
				last_space = 0;
			}
			if (c_str[j] == 32)
				last_space = newline.characters.size();
		}
		colon = -1;
		for (unsigned i = 0; i < newline.characters.size(); i++) {
			if (newline.characters[i].ascii == ':') {
				colon = i;
				break;
			}
		}
		for (int i = 0; i <= colon; i++)
			newline.characters[i].color = color_notify;

		if (!newline.characters.empty())
			lines.push_back(newline);
		newline.Clear();
	}
	queue.clear();
}

void ConsoleLog(const char* message, ...)
{
	static char    msg_text[MAX_MSG_LEN];
	va_list        marker;

	va_start(marker, message);
	vsprintf(msg_text, message, marker);
	va_end(marker);
	queue.push_back(msg_text);
}