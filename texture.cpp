/*-----------------------------------------------------------------------------

  Texture.cpp

  This holds our texture library and the Texture object class.

  (c) 2014 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "file.h"
#include "resource.h"
#include "texture.h"

#define FAIL_SIZE       32
#define DEFAULT_SIZE    8

static unsigned         default_counter;
static vector<Texture*> library;
static int              fail_textures;
static bool             validate_needed;
static vector<int>      texture_stack;

/*-----------------------------------------------------------------------------
Build the raw pixel data for a lo-res checkerboard texture. Half the pixels
will be white, the other half will be a color unique to this texture.
This is used as a fallback when a texture load fails.
-----------------------------------------------------------------------------*/

static char* do_default_image(GLcoord2* size_in)
{
	GLrgba          color;
	unsigned char   bcolor[4];
	unsigned char   white[4];
	int             x, y;
	unsigned char*  buffer;
	unsigned char*  current;

	color = GLrgbaUnique(default_counter++);
	bcolor[0] = (unsigned char)(color.red * 255.0f);
	bcolor[1] = (unsigned char)(color.green * 255.0f);
	bcolor[2] = (unsigned char)(color.blue * 255.0f);
	bcolor[3] = 255;
	memset(white, 255, 4);
	buffer = new unsigned char[DEFAULT_SIZE * DEFAULT_SIZE * 4];
	if (size_in) {
		size_in->x = DEFAULT_SIZE;
		size_in->y = DEFAULT_SIZE;
	}
	for (x = 0; x < DEFAULT_SIZE; x++) {
		for (y = 0; y < DEFAULT_SIZE; y++) {
			current = buffer + (x + y * DEFAULT_SIZE) * 4;
			if ((x + y) % 2)
				memcpy(current, white, 4);
			else
				memcpy(current, bcolor, 4);
		}
	}
	return (char*)buffer;
}

//This loads the image data from disk, but doesn't touch open GL.
void Texture::ImageLoad(const char* filename)
{
	int         ok;
	string      location;

	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	location = ResourceLocation(filename, RESOURCE_TEXTURE);
	ok = ilLoadImage(location.c_str());
	if (!ok) {
		Console("%s not found.", filename);
		_buffer = do_default_image(&_size);
	}
	else {
		_size.x = ilGetInteger(IL_IMAGE_WIDTH);
		_size.y = ilGetInteger(IL_IMAGE_HEIGHT);
		_buffer = new char[_size.x * _size.y * 4];
		ilCopyPixels(0, 0, 0, _size.x, _size.y, 1, IL_RGBA, IL_UNSIGNED_BYTE, _buffer);
	}
}

void Texture::Load()
{
	if (!_glid)
		glGenTextures(1, &_glid);
	Bind();
	if (!_buffer)
		ImageLoad(_filename.c_str());
	glTexImage2D(GL_TEXTURE_2D, 0, 4, _size.x, _size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, _buffer);
	FilterApply();
	//gluBuild2DMipmaps(GL_TEXTURE_2D, 4, _size.x, _size.y, GL_RGBA, GL_UNSIGNED_BYTE, _buffer);
}

void Texture::FilterApply()
{
	Bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::Bind()
{
	glBindTexture(GL_TEXTURE_2D, _glid);
}

Texture::Texture(string name)
{
	_glid = 0;
	_buffer = NULL;
	_name = name;
	_filename = name;
	Load();
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void TexturePush(unsigned id)
{
	int     prev_texture;

	glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_texture);
	texture_stack.push_back((unsigned)prev_texture);
	glBindTexture(GL_TEXTURE_2D, id);
}

void TexturePop()
{
	unsigned  id;

	id = texture_stack[texture_stack.size() - 1];
	texture_stack.pop_back();
	glBindTexture(GL_TEXTURE_2D, id);
}

/*
This is the preferred method for obtaining texture objects. If the same texture is used
multiple times, this will avoid redundant data. If texture data is blown away by some OpenGL
upheval, this will recover it seamlessly. If you instance the Texture class yourself,
you'll have to keep track of it yourself as it won't be part of this managed library.
*/
Texture* TextureFromName(string name)
{
	Texture* t;

	for (unsigned i = 0; i < library.size(); i++) {
		if (!stricmp(library[i]->Name().c_str(), name.c_str()))
			return library[i];
	}
	t = new Texture(name);
	library.push_back(t);
	return t;
}

void Texture::Destroy()
{
	if (_glid)
		glDeleteTextures(1, &_glid);
	_glid = 0;
	if (_buffer)
		delete _buffer;
	_buffer = NULL;
}

Texture* TextureFromName(char* name)
{
	string  sname;
	sname.append(name);
	return TextureFromName(sname);
}

void TextureValidate()
{
	validate_needed = true;
}

void TextureUpdate()
{
	if (!validate_needed)
		return;
	Console ("Reloading %d textures...", library.size ());
	validate_needed = false;
	for (unsigned i = 0; i < library.size(); i++) {
		//if (glIsTexture(library[i]->Id()))
			//continue;
		library[i]->Load();
	}
}