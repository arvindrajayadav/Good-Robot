#ifndef TEXTURE_H
#define TEXTURE_H

class Texture
{
private:
	string          _name;
	string          _filename;
	GLcoord2        _size;
	unsigned        _glid;
	char*           _buffer;

	void            FilterApply();
	void            ImageLoad(const char* filename);

public:
	Texture(string name);

	void            Bind();
	const char*     Data() { return _buffer; }
	void            Destroy();
	unsigned        Id() { return _glid; }
	void            Load();
	string          Name() { return _name; }
	GLcoord2        Size() { return _size; }
};

Texture*  TextureFromName(string name);
Texture*  TextureFromName(char* name);
void      TextureUpdate();
void      TextureValidate();

void      TexturePush(unsigned id);
void      TexturePop();

#endif // TEXTURE_H
