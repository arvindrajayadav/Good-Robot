#ifndef MENU_H
#define MENU_H

enum
{
	MENU_NONE,
	MENU_MAIN,
	MENU_UPGRADE,
	MENU_STORE,
	MENU_GAMEOVER,
	MENU_WIN,
	MENU_HAT,
	MENU_COUNT
};

void        MenuInit();
bool        MenuIsOpen();
int         MenuOpenGet();
void        MenuOpen(int menu);
void        MenuUpdate();
void        MenuReset();
void        MenuRender2D();
void        MouseRender();
void        MenuResize();
void        MenuLoadLeaderboards();

#endif // MENU_H