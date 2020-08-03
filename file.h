#ifndef FILE_H
#define FILE_H

int     FileCopy(const char* from, const char* to);
void    FileDelete(string filename);
bool    FileSave(string filename, const char *buf, int size);
bool    FileExists(string filename);
string  FileContents(string name);
char*   FileContentsBinary(string filename, long* size_in = NULL);

#endif // FILE_H
