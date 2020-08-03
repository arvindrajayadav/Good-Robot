#ifndef CONSOLE_H
#define CONSOLE_H

void ConsoleInit();
void ConsoleInput(int key, int char_code);
bool ConsoleIsOpen();
void ConsoleLog(const char* message, ...);
void ConsoleRender();
void ConsoleToggle();
void ConsoleUpdate();

#endif // CONSOLE_H