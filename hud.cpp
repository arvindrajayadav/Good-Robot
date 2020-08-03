/*-----------------------------------------------------------------------------

  HUD.cpp

  Draw health bars, ammo, and other relevant player info.

  Good Robot
  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"
#include "avatar.h"
#include "entity.h"
#include "env.h"
#include "page.h"
#include "particle.h"
#include "system.h"
#include "world.h"

#define CURSOR_SIZE           0.15f

static vector<SpriteBox>  sprites;
static string             message;
static bool               have_message;
static int                gl_list_hud;
static int                hud_height;
static int                xp_flash_end;
static bool               xp_flashing;
static bool               flash_on;
static GLcoord2           last_screen_size;
static float              flash_fade;
static GLrgba             flash_color;
static int                fps_next;
static int                fps_count_last;
static int                fps_count;
static bool               hud_visible;

//Pyrodactyl stuff
#include "ui_hud.h"

static pyrodactyl::HUD hud_objects;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static void do_stats()
{
	if (!EnvValueb(ENV_INFO))
		return;
	GLcoord2  pg;
	GLvector2 position = PlayerPosition();

	pg.x = (int)position.x / PAGE_SIZE;
	pg.y = (int)position.y / PAGE_SIZE;
	InterfacePrint("Position: %1.1f %1.1f", position.x, position.y);
	InterfacePrint("Page: %d %d", pg.x, pg.y);
	InterfacePrint("TMX: %s", WorldPage(pg)->Pattern());
	InterfacePrint("Checkpoint: Map %d Zone %d Room %d", WorldPlayerLocation().x, WorldPlayerLocation().y, WorldRoomFromPosition(position));
	InterfacePrint("LocationID %d", WorldLocationId());
	InterfacePrint("Zone list: [complete] {current} (undiscovered)");
	InterfacePrint(PlayerZones());
	InterfacePrint("---STATS---");
	InterfacePrint("Damage Taken: %d", Player()->Trivia(TRIVIA_DAMAGE_TAKEN));
	InterfacePrint("Damage Dealt: %d", Player()->Trivia(TRIVIA_DAMAGE_DEALT));
	InterfacePrint("$ gathered: %d", Player()->Trivia(TRIVIA_XP_GATHERED));
	InterfacePrint("Bullets Fired: %d", Player()->Trivia(TRIVIA_BULLETS_FIRED));
	InterfacePrint("Kills: %d", Player()->Trivia(TRIVIA_KILLS));
	InterfacePrint("Deaths: %d", Player()->Trivia(TRIVIA_DEATHS));
	if (Player()->Trivia(TRIVIA_CM_TRAVELED) < CM_PER_KM)
		InterfacePrint("Distance Traveled: %1.2fm", (float)Player()->Trivia(TRIVIA_CM_TRAVELED)*ONE_CENTIMETER);
	else
		InterfacePrint("Distance Traveled: %1.2fkm", (float)Player()->Trivia(TRIVIA_CM_TRAVELED) / CM_PER_KM);
	InterfacePrint ("Missiles Shot Down: %d", Player()->Trivia(TRIVIA_MISSILES_DESTROYED));
	InterfacePrint ("Homing missiles evaded: %d", Player()->Trivia(TRIVIA_MISSILES_EVADED));
	InterfacePrint ("Particles: %d", ParticleCount ());
	InterfacePrint ("All Robots: %d", EntityRobotCount ());
	InterfacePrint ("Dead Robots: %d", EntityRobotsDead ());
	InterfacePrint ("");
}

static void do_fps(const Font* f, GLcoord2 pos)
{
	if (!EnvValueb(ENV_FPS))
		return;
	if (fps_count_last < 50)
		glColor3f(1, 0, 0);
	else if (fps_count_last < 60)
		glColor3f(1, 1, 0);
	else
		glColor3f(0, 1, 0);
	f->Print(pos, StringSprintf("FPS: %d", fps_count_last).c_str());
}

static void do_flash(GLcoord2 size)
{
	GLrgba      color;
	GLuvFrame*  uv;
	float				fade;

	uv = SpriteMapLookup(SPRITE_FLASH);
	glBlendFunc(GL_ONE, GL_ONE);
	fade = clamp (flash_fade * 2.0f, 0.0f, 1.0f);
	color = flash_color * fade;
	flash_fade -= 0.01f;
	glColor3fv(&color.red);
	glBegin(GL_QUADS);
	glTexCoord2fv(&uv->uv[0].x);   glVertex2i(0, 0);
	glTexCoord2fv(&uv->uv[1].x);   glVertex2i(size.x, 0);
	glTexCoord2fv(&uv->uv[2].x);   glVertex2i(size.x, size.y);
	glTexCoord2fv(&uv->uv[3].x);   glVertex2i(0, size.y);

	glTexCoord2fv (&uv->uv[0].x);   glVertex2i (0, 0);
	glTexCoord2fv (&uv->uv[1].x);   glVertex2i (size.x, 0);
	glTexCoord2fv (&uv->uv[2].x);   glVertex2i (size.x, size.y);
	glTexCoord2fv (&uv->uv[3].x);   glVertex2i (0, size.y);
	glEnd ();
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void HudUpdate()
{
	do_stats();
}

void HudFlash(GLrgba color)
{
	flash_color = color;
	flash_fade = 1.0f;
}

int HudHeight()
{
	return hud_height;
}

void HudVisible(bool val)
{
	hud_visible = val;

	if (hud_visible)
		Console("HUD On");
	else
		Console("HUD Off");
}

void  HudToggleVisible()
{
	HudVisible(!hud_visible);
}

void HudInit()
{
	gl_list_hud = glGenLists(1);
	hud_objects.Load("core/data/ui_hud.xml");
	HudVisible(true);
}

void HudSetUI()
{
	hud_objects.SetUI();
}

void HudAddSprite(SpriteBox sb)
{
	sprites.push_back(sb);
}

void HudMessage(string message_in)
{
	message = message_in;
	have_message = true;
}

void HudRender()
{
	GLvector2     aim;

	if (!GameActive())
		return;
	aim = PlayerAim();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthMask(false);
	glBlendFunc(GL_ONE, GL_ONE);
	for (unsigned i = 0; i < sprites.size(); i++)
		sprites[i].Render();
	//render aiming cursor
	if (!PlayerIgnore () && hud_visible)
		RenderQuad(aim, SPRITE_TARGET, GLrgba(1, 1, 1), Env().cursor_size, 0, 0, false);
	RenderQuads();
	glDepthMask(true);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
}

void HudRender2D()
{
	if (!hud_visible) {
		sprites.clear ();
		fps_count++;
		return;
	}
	GLcoord2          size;
	int               width;
	GLcoord2          pos;
	GLcoord2          bar_corner;
	int               bar_top;
	const Font*       font;
	bool              recompile;

	if (!GameActive())
		return;
	recompile = false;
	if (GameTick() < xp_flash_end)
		xp_flashing = true;
	else if (xp_flashing) {
		xp_flashing = false;
		recompile = true;
	}
	if (SystemTick() > fps_next) {
		fps_next = SystemTick() + 1000;
		fps_count_last = fps_count;
		fps_count = 0;
		recompile = true;
	}
	font = InterfaceFont(FONT_HUD);
	bar_top = font->Height() + 4;
	size = RenderViewportSize();
	if (flash_fade > 0)
		do_flash(size);
	if (size != last_screen_size)
		recompile = true;
	last_screen_size = size;
	if (recompile) {
		glNewList(gl_list_hud, GL_COMPILE);
		do_fps(font, GLcoord2(0, size.y - font->Height()));
		glEndList();
	}
	glCallList(gl_list_hud);

	hud_objects.Draw();

	if (have_message) {
		vector<FontChar>  output;

		output = font->Parse(message.c_str());
		width = font->Width(output);
		pos = size / 2;
		pos.y += size.y / 4;
		pos.x -= width / 2;
		font->Print(pos, output);
		have_message = false;
	}
	sprites.clear ();
	fps_count++;
}