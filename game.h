#ifndef GAME_H
#define GAME_H

#define GAME_EXPLOSION_DAMAGE   35

bool              GameActive();
void              GameEnd ();
eGameMode         GameMode(string name);
string            GameModeString(eGameMode g);
int               GameFrame ();
void              GameInit();
void              GameLoad(eGameMode gm);
void              GameNew(eGameMode d);
void              GameNew(int character, eGameMode d);
int               GameShoppingId ();
bool              GamePaused();
void              GameQuit();
void              GameReloadData();
void              GameRender();
bool              GameRunning();
void              GameSave();
void              GameTerm();
int               GameTick();
void              GameUpdate();
string            GameSaveFile(eGameMode gm);

void              GameCommand(const char* message, ...);

#endif // GAME_H