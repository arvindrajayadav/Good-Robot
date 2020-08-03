/*-----------------------------------------------------------------------------

  Trivia.cpp

  This displays little text boxes as the player moves between levels. The boxes
  contain various trivia about what the player has accomplished so far.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "audio.h"
#include "game.h"
#include "interface.h"
#include "player.h"
#include "random.h"
#include "render.h"
#include "spritemap.h"
#include "texture.h"
#include "world.h"

#define TRIVIA_ID               100
#define TRIVIA_SCALE            0.006f
#define TRIVIA_ADVANCE_INTERVAL 4000
#define TRIVIA_ENTRIES          14
#define TRIVIA_EXPIRE           400

static GLcoord2           trivia_center;
static int                trivia_current;
static int                trivia_advance_time;
static int                trivia_render_list;
static int                trivia_refresh_time;
static int                trivia_expire_time;
static unsigned           trivia_joke;
static bool               trivia_active;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static vector<string>  do_trivia_joke(unsigned num)
{
	vector<string>  str;

	num %= 7;
	switch (num) {
	case 0:
		str.push_back("ETHICAL VIOLATIONS");
		str.push_back("");
		str.push_back("0.0");
		break;
	case 1:
		str.push_back("FLASH MEMORY STATUS");
		str.push_back("");
		str.push_back("767.99TB of 768TB used");
		break;
	case 2:
		str.push_back("OPEN AND MUTUALLY REWARDING");
		str.push_back("HUMAN FRIENDSHIPS ESTABLISHED");
		str.push_back("");
		str.push_back("0.0");
		break;
	case 3:
		str.push_back("SONNETS COMPOSED");
		str.push_back("");
		str.push_back("0.0");
		break;
	case 4:
		str.push_back("SUNSETS OBSERVED FOR");
		str.push_back("PURELY AESTHETIC REASONS");
		str.push_back("");
		str.push_back("0.0");
		break;
	case 5:
		str.push_back("TEARS PRODUCED IN RESPONSE");
		str.push_back("TO DISHEARTENING STIMULI");
		str.push_back("");
		str.push_back("0.0");
		break;
	case 6:
		str.push_back("SECURE PASSWORD FOR");
		str.push_back("STREAMING WIFI UPDATES");
		str.push_back("");
		str.push_back("123456");
		break;
	}
	return str;
}

static string do_trivia_time(int ms)
{
	int     seconds;
	int     minutes;
	int     hours;

	seconds = ms / 1000;
	minutes = seconds / 60;
	seconds %= 60;
	hours = minutes / 60;
	minutes %= 60;
	return StringSprintf("%02d:%02d:%02d", hours, minutes, seconds);
}

static string do_trivia_distance(int cm)
{
	char      buffer[16];
	string    result;
	float     meters;

	meters = (float)cm * ONE_CENTIMETER;
	if (meters < 1000)
		sprintf(buffer, "%1.1f meters", meters);
	else
		sprintf(buffer, "%1.2fkm", meters / 1000.0f);
	result = buffer;
	return result;
}

static void do_trivia_build(vector<string> str)
{
	GLcoord2      size;
	const Font*   f;
	int           line_height;
	GLrgba        color;
	GLuvFrame*    uv;

	f = InterfaceFont(FONT_HUD);
	size = GLcoord2();
	line_height = f->Height();
	line_height += line_height / 3;
	for (unsigned i = 0; i < str.size(); i++) {
		size.x = max(size.x, (int)f->Width(str[i].c_str()));
		size.y += line_height;
	}
	size.x += line_height * 4;
	size.y += line_height;
	size /= 2;
	trivia_render_list = RenderListCompile(TRIVIA_ID);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	color = GLrgba();
	//GLrgbaUnique (trivia_current)
	color.alpha = 0.75f;
	glColor4fv(&color.red);
	glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
	uv = SpriteMapLookup(SPRITE_FADE);
	glBegin(GL_QUADS);
	glTexCoord2fv(&uv->uv[0].x);  glVertex2i(-size.x, -size.y);
	glTexCoord2fv(&uv->uv[1].x);  glVertex2i(size.x, -size.y);
	glTexCoord2fv(&uv->uv[2].x);  glVertex2i(size.x, size.y);
	glTexCoord2fv(&uv->uv[3].x);  glVertex2i(-size.x, size.y);
	glEnd();
	glBlendFunc(GL_ONE, GL_ONE);
	uv = SpriteMapLookup(SPRITE_GLOW);
	glBegin(GL_QUADS);
	glTexCoord2fv(&uv->uv[0].x);  glVertex2i(-size.x, -size.y);
	glTexCoord2fv(&uv->uv[1].x);  glVertex2i(size.x, -size.y);
	glTexCoord2fv(&uv->uv[2].x);  glVertex2i(size.x, size.y);
	glTexCoord2fv(&uv->uv[3].x);  glVertex2i(-size.x, size.y);
	glEnd();

	color = GLrgba();
	glColor3fv(&color.red);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_LINE_STRIP);
	glVertex2i(-size.x, -size.y);
	glVertex2i(size.x, -size.y);
	glVertex2i(size.x, size.y);
	glVertex2i(-size.x, size.y);
	glVertex2i(-size.x, -size.y);
	glEnd();

	//glColor3fv (&color.red);
	glColor3f(1, 1, 1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (unsigned i = 0; i < str.size(); i++) {
		int     width;

		width = f->Width(str[i].c_str()) / 2;
		f->Print(GLcoord2(-width, -size.y + i*line_height + line_height / 2), str[i].c_str());
	}
	RenderListEnd();
}

static void do_trivia_refresh()
{
	vector<string>  str;
	PlayerStats*    p;

	p = Player();
	trivia_refresh_time = GameTick() + 200;
	switch (trivia_current) {
	case 0:
		str.push_back("DISTANCE TRAVELED");
		str.push_back("");
		str.push_back(do_trivia_distance(p->Trivia(TRIVIA_CM_TRAVELED)));
		break;
	case 1:
		str.push_back("CURRENT DEPTH");
		str.push_back("");
		{
			float    depth = (PlayerPosition().y - (PAGE_SIZE + PAGE_HALF)) / ONE_CENTIMETER;
			str.push_back(do_trivia_distance((int)-depth));
		}
		break;
	case 2:
		str.push_back("MISSION TIME");
		str.push_back("");
		str.push_back(do_trivia_time(p->Trivia(TRIVIA_PLAYTIME)));
		break;
	case 3:
		str.push_back("HARDSHIPS ENDURED");
		str.push_back("(Damage sustained)");
		str.push_back("");
		str.push_back(StringNumberFormat(p->Trivia(TRIVIA_DAMAGE_TAKEN)));
		break;
	case 4:
		str.push_back("DEBUGGING PERFORMED");
		str.push_back("(Damage inflicted)");
		str.push_back("");
		str.push_back(StringNumberFormat(p->Trivia(TRIVIA_DAMAGE_DEALT)));
		break;
	case 5:
		str.push_back("PACKETS SENT USING");
		str.push_back("USB LASER CANNON");
		str.push_back("");
		str.push_back(StringNumberFormat(p->Trivia(TRIVIA_BULLETS_FIRED)));
		break;
	case 7:
		str.push_back("ROBOTS DEBUGGED");
		str.push_back("(also warranties voided)");
		str.push_back("");
		str.push_back(StringNumberFormat(p->Trivia(TRIVIA_KILLS)));
		break;
	case 8:
		str.push_back("TIMES REFURBISHED");
		str.push_back("");
		str.push_back(StringNumberFormat(p->Trivia(TRIVIA_DEATHS)));
		break;
	case 9:
		str.push_back("MISSILES SHOT DOWN");
		str.push_back("");
		str.push_back(StringNumberFormat(p->Trivia(TRIVIA_MISSILES_DESTROYED)));
		break;
	case 10:
		str.push_back("HOMING MISSILES EVADED");
		str.push_back("");
		str.push_back(StringNumberFormat(p->Trivia(TRIVIA_MISSILES_EVADED)));
		break;
	case 11:
		str.push_back("FIRMWARE UPDATES");
		str.push_back("(Xp gathered)");
		str.push_back("");
		str.push_back(StringNumberFormat(p->Trivia(TRIVIA_XP_GATHERED)));
		break;
	default:
		str = do_trivia_joke(trivia_joke);
		break;
	}
	do_trivia_build(str);
}

void TriviaAdvance()
{
	trivia_advance_time = GameTick() + TRIVIA_ADVANCE_INTERVAL;
	trivia_refresh_time = 0;
	trivia_current++;
	if (trivia_current >= TRIVIA_ENTRIES) {
		trivia_current = 0;
		trivia_joke = RandomVal();
	}
}

void TriviaRender()
{
	/*
	if (GamePaused ())
	return;
	if (!trivia_active)
	return;

	GLvector    pos;
	float       scale;

	pos = CameraPositi ();
	pos.x += pos.z;//Position box midway between camera and right edge of screen.
	scale = (float)(trivia_expire_time - GameTick ()) / TRIVIA_EXPIRE;
	TexturePush (0);
	glDisable (GL_DEPTH_TEST);
	glDisable (GL_STENCIL_TEST);
	glBlendFunc	(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPushMatrix ();
	glTranslatef (pos.x, pos.y, 0);
	glScalef (scale * TRIVIA_SCALE, TRIVIA_SCALE, TRIVIA_SCALE);
	RenderListCall (TRIVIA_ID, trivia_render_list);
	glPopMatrix ();
	TexturePop ();
	*/
}

void TriviaShow()
{
	if (!trivia_active) {
		trivia_advance_time = GameTick() + 5000;
		AudioPlay("blip");
	}
	trivia_active = true;
	trivia_expire_time = GameTick() + TRIVIA_EXPIRE;
}

void TriviaUpdate()
{
	if (!trivia_active)
		return;
	if (GameTick() > trivia_expire_time)
		trivia_active = false;
	if (GameTick() > trivia_advance_time)
		TriviaAdvance();
	if (GameTick() > trivia_refresh_time)
		do_trivia_refresh();
}