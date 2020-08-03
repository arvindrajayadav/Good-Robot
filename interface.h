#ifndef INTERFACE_H
#define INTERFACE_H

#include "font.h"

enum InterfaceFontId
{
	FONT_MENU,
	FONT_FIXEDWIDTH,
	FONT_TITLE,
	FONT_HUD
};

void              InterfaceInit();
void              InterfacePrint(const char* message, ...);
void              InterfaceRender2D();
const class Font* InterfaceFont(const unsigned int id);

#endif // INTERFACE_H