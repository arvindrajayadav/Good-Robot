/*-----------------------------------------------------------------------------

  Resource.cpp

  This module will look for the requested file in the expected directory,
  moving through fallback directories until the file is found.

  Eventually this can be used for modding: It can look for the file under
  the mod directory, and if the requested file isn't found it can look in the
  /core/ directory.

  Good Robot
  (c) 2014 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include "file.h"
#include "resource.h"

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

bool resource_exists(string filename)
{
	std::ifstream ifile(filename.c_str());
	return ifile.is_open();
}

/*-----------------------------------------------------------------------------
Given the filename (with no path) and resource type, return the full
filename and path to the requested file.
-----------------------------------------------------------------------------*/

string ResourceLocation(string filename, ResourceType type)
{
	string    result;
	string    subdir;

	switch (type) {
	case RESOURCE_DATA:
		subdir = "data/"; break;
	case RESOURCE_TEXTURE:
		subdir = "textures/"; break;
	case RESOURCE_SOUND:
		subdir = "sounds/"; break;
	case RESOURCE_SHADER:
		subdir = "shaders/"; break;
	case RESOURCE_MUSIC:
		subdir = "music/"; break;
	}

#ifdef WINDEBUG
	path    p;
	string  dest;

	//Grab files that are under source control and copy them to their intended location.
	if (type == RESOURCE_DATA || type == RESOURCE_TEXTURE || type == RESOURCE_SHADER) {
		result = string(".\\..\\GoodRobot\\") + filename;
		if (resource_exists(result)) {
			p = string(".\\..\\Run\\core\\") + subdir;
			create_directory(p);
			dest = string(".\\..\\Run\\core\\") + subdir + filename;
			FileCopy(result.c_str(), dest.c_str());
			return result;
		}
	}
	if (type == RESOURCE_TEXTURE) {
		result = string(".\\..\\GoodRobot\\") + subdir + filename;
		if (resource_exists(result)) {
			p = string(".\\..\\Run\\core\\") + subdir;
			create_directory(p);
			dest = string(".\\..\\Run\\core\\") + subdir + filename;
			FileCopy(result.c_str(), dest.c_str());
			return result;
		}
	}
	//Grab files NOT under source control
	if (type == RESOURCE_SOUND || type == RESOURCE_MUSIC) {
		result = string(".\\..\\GoodRobot\\") + subdir + filename;
		if (resource_exists(result)) {
			p = string(".\\..\\Run\\core\\") + subdir;
			create_directory(p);
			dest = string(".\\..\\Run\\core\\") + subdir + filename;
			FileCopy(result.c_str(), dest.c_str());
			return result;
		}
	}
#endif
	//TODO: We'll add a mod directory override here. Someday. Maybe.
	result = string("core/") + subdir + filename;
	return result;
}
