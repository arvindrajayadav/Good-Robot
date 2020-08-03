#ifndef RENDER_H
#define RENDER_H

#define STENCIL_OCCLUSION 1
#define STENCIL_LAMP      2

void      Render();
float     RenderAspect ();
void      RenderGlow ();
void      RenderInit();
void      RenderCircularBar(class Texture* t, int start_angle, int end_angle, float radius, GLcoord2 pos_in);
void      RenderCircularBar (float start_angle, float end_angle, float radius, GLvector2 pos, SpriteEntry sprite);
void      RenderCompile();
int       RenderListCompile(int owner);
void      RenderListEnd();
void      RenderListCall(int owner, int index);
void      RenderOverlay(GLcoord2 pos, GLcoord2 size, int texture, float intensity);
inline bool      RenderPointVisible (GLvector2 point);
void      RenderPushViewport(int width, int height);
void      RenderPopViewport();
void      RenderQuads();
void      RenderQuad (GLvector2 pos, enum SpriteEntry sprite, GLrgba color, GLvector2 size, float angle, float depth, bool glow, float blink);
void      RenderQuad (GLvector2 pos, enum SpriteEntry sprite, GLrgba color, float size, float angle, float depth, bool glow);
void      RenderQuad (GLvector2 pos, enum SpriteEntry sprite, GLrgba color, GLvector2 size, float angle, float depth, bool glow);
void      RenderQuadNow(GLvector2 pos, SpriteEntry sprite, GLrgba color, float size, float angle, float depth, bool glow);
void			RenderTriangle (GLvector* v);
void			RenderTriangles ();
GLcoord2  RenderViewportSize();
void      RenderResize(int width, int height);
void      RenderTexture(class Texture* t, GLcoord2 c);
void      RenderTexture(class Texture* t, GLcoord2 c, GLcoord2 size);
void      RenderTexture(class Texture* t, int x, int y);
void      RenderTexture(class Texture* t, int x, int y, SDL_Rect clip);
void      RenderTexture(class Texture* t, const int x, const int y, const int w, const int h, const SDL_Rect* clip = nullptr);
void      RenderTexture(class Texture* t, const int x, const int y, const int w, const int h, const float alpha, const SDL_Rect* clip = nullptr);
void      RenderSprite (const int x, const int y, const int w, const int h, SpriteEntry s, GLrgba color=GLrgba(1,1,1));


void      RenderStencilImprint (unsigned mask);
void      RenderStencilMask (unsigned compare, unsigned mask);

void      RenderWrite (bool write);

#endif // RENDER_H
