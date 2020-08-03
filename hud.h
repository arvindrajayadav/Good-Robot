#ifndef HUD_H
#define HUD_H

#include "bodyparts.h"

void  HudAddSprite(SpriteBox sb);
void  HudFlash(GLrgba color);
int   HudHeight();
void  HudInit();
void  HudSetUI();
void  HudMessage(string message_in);
void  HudVisible(bool val);
void  HudToggleVisible();
void  HudRender();
void  HudRender2D();
void  HudUpdate();

#endif // HUD_H
