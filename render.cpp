/*-----------------------------------------------------------------------------

  Render.cpp

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "avatar.h"
#include "camera.h"
#include "file.h"
#include "font.h"
#include "hud.h"
#include "input.h"
#include "interface.h"
#include "main.h"
#include "menu.h"
#include "noise.h"
#include "particle.h"
#include "player.h"
#include "random.h"
#include "resource.h"
#include "sprite.h"
#include "system.h"
#include "texture.h"
#include "world.h"
#include "vectors.h"

#define RENDER_DISTANCE     50
#define NEAR_CLIP           0.2f
#define FOV                 120
#define DELIMIT             "\n\r"
#define MAX_SCRATCH         40
  //STRI is a simple triangle. No color. No texture. It's used for
  //rendering spider legs. This is a limit on how many
  //such triangles we can queue at once.
#define MAX_STRI  					1000

#define TEX_MIN             0.01f
#define TEX_MAX             0.98f

#define VERT_SHADER         "vertex.cg"
#define FRAG_SHADER         "fragment.cg"

struct Qquad
{
	GLvector2   position;
	SpriteEntry sprite;
	GLrgba      color;
	GLvector2   size;
	float       depth;
	float       angle;
	float       blink;
};

struct ScratchList
{
	int         gl_list;
	int         owner;
};

struct Quad
{
	GLvector    corner[4];
};

static int                view_width;
static int                view_height;
static float              view_aspect;
static GLbbox2            view_bbox;
static vector<Qquad>      quad;
static vector<Qquad>      quad_glow;
static unsigned           quad_count;
static unsigned           quad_glow_count;
static bool               rendering_2d;
static VBO                one_quad;
static GLmesh             one_quad_mesh;

static GLenum             my_program;
static GLenum             my_vertex_shader;
static GLenum             my_fragment_shader;

static ScratchList        scratch_list[MAX_SCRATCH];
static int                current_scratch;

static int                quads_this_frame;

static GLvector						stri_vert[MAX_STRI * 3];
static unsigned						stri_index[MAX_STRI * 3];
static int								stri_count;

static unsigned           id_quad_color;
static unsigned           id_quad_position;
static unsigned           id_quad_uv;
static unsigned           id_quad_normal;
static unsigned           id_quad_index;

static vector<GLrgba>     quad_color;
static vector<Quad>       quad_position;
static vector<GLvector>   quad_normal;
static vector<GLvector2>  quad_uv;
static vector<int>        quad_index;

static GLvector2          radial[360];
static GLvector2          radial_uv[360];

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void compile(char* filename, GLenum id)
{
	char*       parse;
	const char* source;
	char        err[512];
	int         len = 512;
	string      text;

	text = FileContents(ResourceLocation(filename, RESOURCE_SHADER));
	if (text.empty()) {
		Console("Can't open '%s'...", filename);
		return;
	}
	source = text.c_str();
	len = text.length();
	glShaderSourceARB(id, 1, (const GLcharARB**)&source, &len);
	glCompileShader(id);
	err[0] = 0;
	glGetInfoLogARB(id, 512, &len, err);

	parse = strtok(err, DELIMIT);
	if (parse) {
		Console("Compiling '%s'...", filename);
		while (parse != NULL) {
			Console(parse);
			parse = strtok(NULL, DELIMIT);
		}
	}
	else
		Console("Compiling '%s'... ok.", filename);
}

static void shader_init()
{
	//Set up our shaders
	my_program = glCreateProgramObjectARB();
	//glCreateShader (
	my_vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	my_fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	compile(VERT_SHADER, my_vertex_shader);
	compile(FRAG_SHADER, my_fragment_shader);
	glAttachObjectARB(my_program, my_vertex_shader);
	glAttachObjectARB(my_program, my_fragment_shader);
	glUseProgramObjectARB(0);
	glLinkProgramARB(my_program);
	glUseProgramObjectARB(my_program);
	//Set up the lone quad used by the shader
	one_quad_mesh.Clear();
	one_quad_mesh.PushVertex(GLvector(-0.5f, -0.5f, 0), GLvector2(TEX_MIN, TEX_MIN));
	one_quad_mesh.PushVertex(GLvector(0.5f, -0.5f, 0), GLvector2(TEX_MAX, TEX_MIN));
	one_quad_mesh.PushVertex(GLvector(0.5f, 0.5f, 0), GLvector2(TEX_MAX, TEX_MAX));
	one_quad_mesh.PushVertex(GLvector(-0.5f, 0.5f, 0), GLvector2(TEX_MIN, TEX_MAX));
	one_quad_mesh.PushQuad(0, 1, 2, 3);
	one_quad.Create(&one_quad_mesh);
}

bool use_shader()
{
	return EnvValueb(ENV_SHADER) && !rendering_2d;
}

static  GLint       attrib_angle;
static  GLint       attrib_scale;
static  GLint       attrib_position;
static  GLint       attrib_atlas;
static  GLint       attrib_blink;

inline void draw_quad(const Qquad* q)
{
	if (use_shader()) {
		GLvector        position;
		const AtlasRef* atlas_ref;

		atlas_ref = SpriteAtlasRef(q->sprite);
		glColor4fv(&q->color.red);
		position = GLvector(q->position.x, q->position.y, q->depth);
		glVertexAttrib1f(attrib_angle, q->angle);
		glVertexAttrib1f(attrib_blink, q->blink);
		glVertexAttrib2fv(attrib_scale, &q->size.x);
		glVertexAttrib3fv(attrib_position, &position.x);
		glVertexAttrib3fv(attrib_atlas, &atlas_ref->col);
		one_quad.Render();
	}
	else { //draw quads in immediate mode
		GLuvFrame*  body;
		GLquad      rect;

		rect = SpriteMapQuad((int)q->angle);
		body = SpriteMapLookup(q->sprite);
		glColor4fv(&q->color.red);
		glBegin(GL_QUADS);
		glTexCoord2fv(&body->uv[2].x);  glVertex3f(q->position.x + rect.corner[0].x * q->size.x, q->position.y + rect.corner[0].y * q->size.y, q->depth);
		glTexCoord2fv(&body->uv[3].x);  glVertex3f(q->position.x + rect.corner[1].x * q->size.x, q->position.y + rect.corner[1].y * q->size.y, q->depth);
		glTexCoord2fv(&body->uv[0].x);  glVertex3f(q->position.x + rect.corner[2].x * q->size.x, q->position.y + rect.corner[2].y * q->size.y, q->depth);
		glTexCoord2fv(&body->uv[1].x);  glVertex3f(q->position.x + rect.corner[3].x * q->size.x, q->position.y + rect.corner[3].y * q->size.y, q->depth);
		glEnd();
	}
}

/*-----------------------------------------------------------------------------
Strencil operations. OpenGL stencil system is powerful, but also cluttered,
verbose, and occasionally obtuse. These functions encapsulate the system.
-----------------------------------------------------------------------------*/

//Write to the stencil buffer.
void RenderStencilImprint(unsigned mask)
{
	glEnable(GL_STENCIL_TEST);
	//Enable writing to these stencil bits...
	//glStencilMask (0xff);
	glStencilMask(mask);
	//The comparison logic to use...
	glStencilFunc(GL_ALWAYS, mask, 0xFF);
	//Actions to take on stencil fail, z fail, z pass.
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	//glColorMask (false, false, false, false);
	//glDepthMask (false);
}

//Mask out future polygons if they don't land on the given masks.
void RenderStencilMask(unsigned compare, unsigned mask)
{
	if (!EnvValueb(ENV_SHADOWS))
		return;
	glStencilMask(0x00);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_EQUAL, compare, mask);
}

void RenderWrite(bool write)
{
	glColorMask(write, write, write, write);
	glDepthMask(write);
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

//Scratch lists are OpenGL lists. We keep them here so we don't have to worry
//about allocating / freeing them in all the little places where they are used.
//The drawback is that if we use a lot at once, a list will get pulled out from
//under the client.  This is harmless, aside from the thing vanishing.
//For this reason, this should only be used for non-critical visual effects.

int RenderListCompile(int owner_id)
{
	int   index;

	index = current_scratch;
	scratch_list[index].owner = owner_id;
	glNewList(scratch_list[index].gl_list, GL_COMPILE);
	current_scratch = (current_scratch + 1) % MAX_SCRATCH;
	return index;
}

void RenderListEnd()
{
	glEndList();
}

void RenderListCall(int owner, int index)
{
	//if the owners don't match, then this list was stolen by another object.
	if (scratch_list[index].owner != owner)
		return;
	glCallList(scratch_list[index].gl_list);
}

void RenderQuads()
{
	GLboolean   depth_mask;

	glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
	if (use_shader()) {
		glUseProgramObjectARB(my_program);
		attrib_angle = glGetAttribLocation(my_program, "attrib_angle");
		attrib_blink = glGetAttribLocation(my_program, "attrib_blink");
		attrib_scale = glGetAttribLocation(my_program, "attrib_scale");
		attrib_position = glGetAttribLocation(my_program, "attrib_position");
		attrib_atlas = glGetAttribLocation(my_program, "attrib_atlas");
	}

	if (quad_count) {
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glDepthMask(true);
		for (unsigned i = 0; i < quad_count; i++)
			draw_quad(&quad[i]);
	}
	if (quad_glow_count) {
		glDepthMask(false);
		glBlendFunc(GL_ONE, GL_ONE);
		for (unsigned i = 0; i < quad_glow_count; i++)
			draw_quad(&quad_glow[i]);
	}
	glDepthMask(depth_mask);
	quad_count = 0;
	quad_glow_count = 0;
	glUseProgramObjectARB(0);
	if (EnvValueb(ENV_SHADER)) {
		glVertexAttrib1f(attrib_angle, 0);
		glVertexAttrib1f(attrib_scale, 1);
		glVertexAttrib3f(attrib_position, 0, 0, 0);
		glVertexAttrib3f(attrib_atlas, 0, 0, -1);
	}
}

void RenderQuad(GLvector2 pos, SpriteEntry sprite, GLrgba color, GLvector2 size, float angle, float depth, bool glow, float blink)
{
	Qquad*    q;

	quads_this_frame++;
	if (glow) {
		if (quad_glow_count >= quad_glow.size()) {
			Qquad   q;

			quad_glow.push_back(q);
		}
		q = &quad_glow[quad_glow_count];
	}
	else {
		if (quad_count >= quad.size()) {
			Qquad   q;

			quad.push_back(q);
		}
		q = &quad[quad_count];
	}
	q->blink = blink;
	q->position = pos;
	q->sprite = sprite;
	q->color = color;
	q->size = size;
	q->angle = angle;
	q->depth = depth;
	if (glow)
		quad_glow_count++;
	else
		quad_count++;
}

void RenderQuad(GLvector2 pos, SpriteEntry sprite, GLrgba color, GLvector2 size, float angle, float depth, bool glow)
{
	RenderQuad(pos, sprite, color, size, angle, depth, glow, 0.0f);
}

void RenderQuad(GLvector2 pos, SpriteEntry sprite, GLrgba color, float size, float angle, float depth, bool glow)
{
	RenderQuad(pos, sprite, color, GLvector2(size, size), angle, depth, glow, 0.0f);
}

void RenderQuadNow(GLvector2 pos, SpriteEntry sprite, GLrgba color, float size, float angle, float depth, bool glow)
{
	Qquad   q;

	if (glow)
		glBlendFunc(GL_ONE, GL_ONE);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	q.position = pos;
	q.sprite = sprite;
	q.color = color;
	q.size = GLvector2(size, size);
	q.angle = angle;
	q.depth = depth;
	draw_quad(&q);
	quads_this_frame++;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

GLcoord2 RenderViewportSize()
{
	return GLcoord2(view_width, view_height);
}

void RenderPushViewport(int width, int height)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width, height, 0, 0.1f, 2048);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0, 0, -1.0f);
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_FOG);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(false);
	glDisable(GL_DEPTH_TEST);
	glColor3f(1, 1, 1);
}

void RenderPopViewport()
{
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

float RenderAspect()
{
	return view_aspect;
}

void RenderResize(int width, int height)
{
	float       fovy;
	int         size;

	Console("RenderCreate: Creating %dx%d viewport", width, height);
	//Purge the front and back buffers so the user doesn't see all the messy
	//artifacts when the screen is re-sized.
	glClearColor(0.0f, 0, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	SystemSwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	SystemSwapBuffers();

	view_width = width;
	view_height = height;
	view_aspect = (float)width / (float)height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	fovy = FOV;
	if (view_aspect > 1.0f)
		fovy /= view_aspect;
	glViewport(0, 0, width, height);
	gluPerspective(fovy, view_aspect, NEAR_CLIP, RENDER_DISTANCE);
	glMatrixMode(GL_MODELVIEW);
	size = min(width, height);
	if (my_vertex_shader)
		shader_init();
	TextureValidate();
	FontValidate();
	WorldValidate();
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void RenderTexture(Texture* t, GLcoord2 pos, GLcoord2 size)
{
	GLcoord2    end;

	end = pos + size;
	glDepthMask(false);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, t->Id());
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1);  glVertex2i(pos.x, pos.y);
	glTexCoord2f(1, 1);  glVertex2i(end.x, pos.y);
	glTexCoord2f(1, 0);  glVertex2i(end.x, end.y);
	glTexCoord2f(0, 0);  glVertex2i(pos.x, end.y);
	glEnd();
}

void RenderTexture(Texture* t, GLcoord2 c)
{
	RenderTexture(t, c, t->Size());
}

void RenderTexture(Texture* t, int x, int y)
{
	GLcoord2 c(x, y);
	RenderTexture(t, c, t->Size());
}

void RenderTexture(Texture* t, int x, int y, SDL_Rect clip)
{
	GLcoord2 pos(x, y);
	GLcoord2 end(pos.x + clip.w, pos.y + clip.h);

	float start_x = clip.x / (float)t->Size().x;
	float start_y = clip.y / (float)t->Size().y;

	float end_x = (clip.x + clip.w) / (float)t->Size().x;
	float end_y = (clip.y + clip.h) / (float)t->Size().y;

	glDepthMask(false);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, t->Id());
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin(GL_QUADS);
	glTexCoord2f(start_x, end_y);  glVertex2i(pos.x, pos.y);
	glTexCoord2f(end_x, end_y);  glVertex2i(end.x, pos.y);
	glTexCoord2f(end_x, start_y);  glVertex2i(end.x, end.y);
	glTexCoord2f(start_x, start_y);  glVertex2i(pos.x, end.y);
	glEnd();
}

void RenderTexture(class Texture* t, const int x, const int y, const int w, const int h, const SDL_Rect* clip)
{
	GLcoord2 pos(x, y), size(w, h);
	GLcoord2 end = pos + size;

	glDepthMask(false);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, t->Id());
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (clip == nullptr) {
		//Draw the full texture
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);  glVertex2i(pos.x, pos.y);
		glTexCoord2f(1, 1);  glVertex2i(end.x, pos.y);
		glTexCoord2f(1, 0);  glVertex2i(end.x, end.y);
		glTexCoord2f(0, 0);  glVertex2i(pos.x, end.y);
		glEnd();
	}
	else {
		float tex_start_x = clip->x / (float)t->Size().x;
		float tex_start_y = clip->y / (float)t->Size().y;

		float tex_end_x = (clip->x + clip->w) / (float)t->Size().x;
		float tex_end_y = (clip->y + clip->h) / (float)t->Size().y;

		glBegin(GL_QUADS);
		glTexCoord2f(tex_start_x, tex_end_y);    glVertex2i(pos.x, pos.y);
		glTexCoord2f(tex_end_x, tex_end_y);      glVertex2i(end.x, pos.y);
		glTexCoord2f(tex_end_x, tex_start_y);    glVertex2i(end.x, end.y);
		glTexCoord2f(tex_start_x, tex_start_y);  glVertex2i(pos.x, end.y);
		glEnd();
	}
}

void RenderTexture(class Texture* t, const int x, const int y, const int w, const int h, const float alpha, const SDL_Rect* clip = nullptr)
{
	GLcoord2 pos(x, y), size(w, h);
	GLcoord2 end = pos + size;

	glDepthMask(false);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glColor4f(alpha, alpha, alpha, alpha);
	glBindTexture(GL_TEXTURE_2D, t->Id());
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (clip == nullptr) {
		//Draw the full texture
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);  glVertex2i(pos.x, pos.y);
		glTexCoord2f(1, 1);  glVertex2i(end.x, pos.y);
		glTexCoord2f(1, 0);  glVertex2i(end.x, end.y);
		glTexCoord2f(0, 0);  glVertex2i(pos.x, end.y);
		glEnd();
	}
	else {
		float tex_start_x = clip->x / (float)t->Size().x;
		float tex_start_y = clip->y / (float)t->Size().y;

		float tex_end_x = (clip->x + clip->w) / (float)t->Size().x;
		float tex_end_y = (clip->y + clip->h) / (float)t->Size().y;

		glBegin(GL_QUADS);
		glTexCoord2f(tex_start_x, tex_end_y);    glVertex2i(pos.x, pos.y);
		glTexCoord2f(tex_end_x, tex_end_y);      glVertex2i(end.x, pos.y);
		glTexCoord2f(tex_end_x, tex_start_y);    glVertex2i(end.x, end.y);
		glTexCoord2f(tex_start_x, tex_start_y);  glVertex2i(pos.x, end.y);
		glEnd();
	}
}

void RenderSprite(const int x, const int y, const int w, const int h, SpriteEntry s, GLrgba color)
{
	GLuvFrame*    uv = SpriteMapLookup(s);
	GLcoord2 pos(x, y), size(w, h);
	GLcoord2 end = pos + size;

	glDepthMask(false);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glColor3fv(&color.red);
	glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBegin(GL_QUADS);
	glTexCoord2f(uv->uv[0].x, uv->uv[0].y);  glVertex2i(pos.x, pos.y);
	glTexCoord2f(uv->uv[1].x, uv->uv[1].y);  glVertex2i(end.x, pos.y);
	glTexCoord2f(uv->uv[2].x, uv->uv[2].y);  glVertex2i(end.x, end.y);
	glTexCoord2f(uv->uv[3].x, uv->uv[3].y);  glVertex2i(pos.x, end.y);
	glEnd();
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

bool RenderPointVisible(GLvector2 point)
{
	return view_bbox.Contains(point);
}

void RenderOverlay(GLcoord2 pos, GLcoord2 size, int texture, float intensity)
{
	GLuvFrame*      uv_screen;
	GLuvFrame*      uv_scan;
	GLuvFrame       uv_snow;
	GLcoord2        end;

	glDepthMask(false);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	//glBindTexture (GL_TEXTURE_2D, 0);
	end = pos + size;
	uv_screen = SpriteMapLookup(SPRITE_SCREEN);
	uv_scan = SpriteMapLookup(SPRITE_SCANLINES);
	uv_snow = *SpriteMapLookup(SPRITE_SNOW);
	if (texture) {
		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 1);  glVertex2i(pos.x, pos.y);
		glTexCoord2f(1, 1);  glVertex2i(end.x, pos.y);
		glTexCoord2f(1, 0);  glVertex2i(end.x, end.y);
		glTexCoord2f(0, 0);  glVertex2i(pos.x, end.y);
		glEnd();
	}
	glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
	{
		GLvector2   snow_size;
		GLvector2   ul, lr;

		snow_size = uv_snow.Size() / 2;
		ul = uv_snow.uv[0] - snow_size * (GLvector2(RandomFloat(), RandomFloat()) / 2);
		lr = ul - snow_size;
		ul.y = 1 - ul.y;
		lr.y = 1 - lr.y;
		uv_snow.Set(ul, lr);
		glColor3f(intensity, intensity, intensity);
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
		glBegin(GL_QUADS);
		glTexCoord2fv(&uv_snow.uv[0].x);  glVertex2i(pos.x, pos.y);
		glTexCoord2fv(&uv_snow.uv[1].x);  glVertex2i(end.x, pos.y);
		glTexCoord2fv(&uv_snow.uv[2].x);  glVertex2i(end.x, end.y);
		glTexCoord2fv(&uv_snow.uv[3].x);  glVertex2i(pos.x, end.y);
		glEnd();
	}
	{
		int   syncline;
		float delta;
		int   linepos;
		float v;

		glLineWidth(4.0f);
		syncline = SystemTick() % 5000;
		delta = ((float)syncline / 5000.0f);
		linepos = pos.y + (int)(delta * (float)size.y);
		v = Lerp(uv_scan->uv[2].y, uv_scan->uv[0].y, delta);
		glColor3f(0.3f, 0.3f, 0.3f);
		glBlendFunc(GL_ONE, GL_ONE);
		glBegin(GL_LINES);
		glTexCoord2f(uv_scan->uv[0].x, v);
		glVertex2i(pos.x, linepos);
		glTexCoord2f(uv_scan->uv[1].x, v);
		glVertex2i(end.x, linepos);
		glEnd();
	}
	glColor3f(intensity, intensity, intensity);
	glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	glBegin(GL_QUADS);
	glTexCoord2fv(&uv_scan->uv[0].x);  glVertex2i(pos.x, pos.y);
	glTexCoord2fv(&uv_scan->uv[1].x);  glVertex2i(end.x, pos.y);
	glTexCoord2fv(&uv_scan->uv[2].x);  glVertex2i(end.x, end.y);
	glTexCoord2fv(&uv_scan->uv[3].x);  glVertex2i(pos.x, end.y);
	glEnd();
	glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
	glColor3f(1, 1, 1);
	glBegin(GL_QUADS);
	glTexCoord2fv(&uv_screen->uv[0].x);  glVertex2i(pos.x, pos.y);
	glTexCoord2fv(&uv_screen->uv[1].x);  glVertex2i(end.x, pos.y);
	glTexCoord2fv(&uv_screen->uv[2].x);  glVertex2i(end.x, end.y);
	glTexCoord2fv(&uv_screen->uv[3].x);  glVertex2i(pos.x, end.y);
	glEnd();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void RenderCircularBar(float start_angle, float end_angle, float radius, GLvector2 pos, SpriteEntry sprite)
{
	GLuvFrame*    uv = SpriteMapLookup(sprite);
	GLvector2     center, uv_edge;
	float         uv_radius;

	center = (uv->uv[0] + uv->uv[2]) / 2.0f;
	uv_radius = uv->Size().x / 2.0f;
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2fv(&center.x);
	glVertex2fv(&pos.x);
	if (end_angle < start_angle)
		end_angle += 360;
	for (int a = (int)start_angle; a <= (int)end_angle; a++) {
		int   angle = a % 360;
		if (angle < 0)
			angle = 360 + angle;
		uv_edge = center + (radial[angle] * uv_radius);
		glTexCoord2fv(&uv_edge.x);
		glVertex2f(pos.x - radial[angle].x*radius, pos.y - radial[angle].y*radius);
	}
	glEnd();
}

void RenderCircularBar(Texture* t, int start_angle, int end_angle, float radius, GLcoord2 pos_in)
{
	GLvector2 pos;

	pos = pos_in;
	t->Bind();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor3f(1, 1, 1);

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5f, 0.5f);
	glVertex2fv(&pos.x);
	if (end_angle < start_angle)
		end_angle += 360;
	for (int a = start_angle; a <= end_angle; a++) {
		int   angle = a % 360;
		if (angle < 0)
			angle = 360 + angle;
		glTexCoord2fv(&radial_uv[angle].x);
		glVertex2f(pos.x + radial[angle].x*radius, pos.y + radial[angle].y*radius);
	}
	glEnd();
}

void RenderCompile()
{
	shader_init();
}

void RenderInit()
{
	shader_init();
	//Init our scratch lists
	for (int i = 0; i < MAX_SCRATCH; i++)
		scratch_list[i].gl_list = glGenLists(1);
	for (int a = 0; a < 360; a++) {
		radial[a] = GLvectorFromAngle((float)a);
		radial_uv[a] = (radial[a] / 2.0f) + 0.5f;
	}
	glGenBuffers(1, &id_quad_color);
	glGenBuffers(1, &id_quad_position);
	glGenBuffers(1, &id_quad_uv);
	glGenBuffers(1, &id_quad_normal);
	glGenBuffers(1, &id_quad_index);
	quad_color.resize(5000);
	quad_position.resize(5000);
	quad_uv.resize(5000);
	quad_normal.resize(5000);
	quad_index.resize(5000 * 4);
	for (int i = 0; i < MAX_STRI * 3; i++) {
		stri_index[i] = i;
	}
}

void Render()
{
	GLcoord2  s;

	{
		GLvector  camera_current = CameraPosition();
		GLvector  limit;

		limit.y = camera_current.z;
		limit.x = limit.y * RenderAspect();
		limit *= 0.95f;
		view_bbox.Clear();
		view_bbox.ContainPoint(GLvector2(camera_current.x - limit.x, camera_current.y - limit.y));
		view_bbox.ContainPoint(GLvector2(camera_current.x + limit.x, camera_current.y + limit.y));
	}
#ifdef _WIN32
	wglSwapIntervalEXT(EnvValueb(ENV_VSYNC));
#endif

	quads_this_frame = 0;
	//Set up all the different OpenGL state variables.
	glStencilMask(0xff);
	glClearStencil(0);
	glStencilMask(0x0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glLineWidth(3.0f);
	glPointSize(4.0f);
	glAlphaFunc(GL_GREATER, 0.05f);
	glEnable(GL_ALPHA_TEST);
	glMatrixMode(GL_PROJECTION);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glLoadIdentity();
	gluPerspective(90, view_aspect, 0.01, 1000);
	glMatrixMode(GL_MODELVIEW);
	if (EnvValueb(ENV_RENDER_WIREFRAME))
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	GameRender();

	s = RenderViewportSize();
	RenderPushViewport(s.x, s.y);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glUseProgramObjectARB(0);
	rendering_2d = true;
	WorldRender2D();
	HudRender2D();
	MenuRender2D();
	InterfaceRender2D();
	MouseRender();
	RenderPopViewport();
	ConsoleRender();
	rendering_2d = false;
	glUseProgramObjectARB(my_program);
}

void	RenderTriangle(GLvector* v)
{
	if (stri_count >= MAX_STRI)
		return;
	int		current_vert = stri_count * 3;
	stri_vert[current_vert + 0] = v[0];
	stri_vert[current_vert + 1] = v[1];
	stri_vert[current_vert + 2] = v[2];
	stri_count++;
}

void	RenderTriangles()
{
	if (stri_count == 0)
		return;
	glFlush();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(0, 0, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//Enable the vertex array functionality:
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(GLvector), stri_vert);
	glDrawElements(GL_TRIANGLES, //mode
		stri_count * 3,  //count, ie. how many indices
		GL_UNSIGNED_INT, //type of the index array
		stri_index);
	glDisableClientState(GL_VERTEX_ARRAY);
	stri_count = 0;
	glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
}