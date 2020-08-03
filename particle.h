#ifndef PARTICLE_H
#define PARTICLE_H

void			ParticleBlood (GLvector2 origin, GLrgba color, float size, int count, float speed, GLvector2 direction);
void      ParticleBloom(GLvector2 origin, GLrgba color, float size, int lifespan);
unsigned  ParticleCount();
void      ParticleInit();
void      ParticleDebris(GLvector2 origin, float size, int count, float speed = 1);
void      ParticleDebris (GLvector2 origin, float size, int count, float speed, GLvector2 impact_direction);
void      ParticleExplode (GLvector2 origin, GLrgba color, int count, float size);
void      ParticleRender();
void      ParticleRubble(GLvector2 origin, float size, int count);
void      ParticleSparks(GLvector2 origin, GLrgba color, int count);
void      ParticleSparks(GLvector2 origin, GLvector2 movement, GLrgba color, int count);
void      ParticleSprite(GLvector2 origin, GLvector2 movement, GLrgba color, SpriteEntry sprite, int count, float size, bool glow = true);
void      ParticleSmoke(GLvector2 origin, float size, int count);
void      ParticleGlow(GLvector2 origin, GLvector2 movement, GLrgba color1, GLrgba color2, int count, float size = 0.5f);
void      ParticleUpdate();

#endif // PARTICLE_H
