#ifndef SYSTEM_H
#define SYSTEM_H

string          SystemConfigFile();
void            SystemInit();
void            SystemGrab();
GLcoord2        SystemMouse();
int             SystemResolutionIndex();
void            SystemResolutionSet(int index);
GLcoord2				SystemResolution (int i);
int							SystemResolutions ();
void						SystemRumble (float strength, int time_ms);
string          SystemSavePath();
GLcoord2        SystemSize();
void            SystemSwapBuffers();
void            SystemTerm();
void            SystemThread(char* name, int (SDLCALL *fn)(void *), void *data);
long            SystemTick();
int             SystemTime();
void            SystemUpdate();
void						SystemSizeWindow ();

#endif // SYSTEM_H
