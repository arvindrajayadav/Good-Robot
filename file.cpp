/*-----------------------------------------------------------------------------

  File.cpp

  Various useful file i/o functions.

  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void FileDelete (string filename)
{ 
  _unlink (filename.c_str ());
}

bool FileExists(string filename)
{
	return exists(filename);
}

bool FileSave (string filename, const char *buf, int size)
{
	std::ofstream ofs(filename.c_str(), std::ofstream::binary | std::ofstream::out | std::ofstream::trunc);
	//ofs.open (filename, );
	if (!ofs.is_open())
		return false;
	ofs.write(buf, size);
	ofs.close();
	return true;
}

string  FileContents(string filename)
{
	std::ifstream ifs(filename.c_str());
	ifs.open(filename.c_str(), std::ifstream::in);
	if (!ifs.is_open())
		Console("File not found: %s", filename.c_str());
	string contents((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	ifs.close();
	return contents;
}

char* FileContentsBinary(string filename, long* size_in)
{
	std::ifstream ifs;
	long          size;
	char*         buffer;

	ifs.open(filename.c_str(), std::ifstream::in | std::ifstream::binary | ios::ate);
	if (!ifs.is_open())
		return NULL;
	size = (long)ifs.tellg();
	ifs.seekg(0);
	buffer = (char*)malloc(size);
	ifs.read(buffer, size);
	ifs.close();
	if (size_in)
		*size_in = size;
	return buffer;
}

int FileCopy(const char* from, const char* to)
{
	std::ifstream initialFile(from, ios::in | ios::binary);
	std::ofstream outputFile(to, ios::out | ios::binary);
	//defines the size of the buffer
	initialFile.seekg(0, ios::end);
	long fileSize = (long)initialFile.tellg();
	//Requests the buffer of the predefined size

	//As long as both the input and output files are open...
	if (initialFile.is_open() && outputFile.is_open()) {
		short* buffer = new short[fileSize];
		//Determine the file's size
		//Then starts from the beginning
		initialFile.seekg(0, ios::beg);
		//Then read enough of the file to fill the buffer
		initialFile.read((char*)buffer, fileSize);
		//And then write out all that was read
		outputFile.write((char*)buffer, fileSize);
		delete[] buffer;
	}
	else if (!outputFile.is_open())  {
		Console("FileCopy: Failed to open '%s'.", to);
		return 0;
	}
	else if (!initialFile.is_open()) {
		Console("FileCopy: Failed to open '%s'.", from);
		return 0;
	}
	initialFile.close();
	outputFile.close();
	return 1;
}

bool FileDelete(char* name)
{
	if (!_unlink(name))
		return true;
	return false;
}
