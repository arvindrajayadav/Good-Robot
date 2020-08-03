#ifndef VISIBLE_H
#define VISIBLE_H

void  VisibleInit();
void  VisibleInvert(bool invert);
float VisibleRadius();
void  VisibleRender();
void  VisibleRenderFog();
void  VisibleRenderCone (float intensity, float depth);
void	VisibleRenderLOS ();
void  VisibleUpdate();

#endif // VISIBLE_H
