//=============================================================================
// Author:   Arvind
// Purpose:  Contains the image manager class - used to manage in-game images
//=============================================================================
#include "master.h"
#include "ImageManager.h"

using namespace pyrodactyl;

//Stuff we use throughout the game
namespace pyrodactyl
{
	ImageManager gImageManager;
}

//------------------------------------------------------------------------
// Purpose: Load assets here.
//------------------------------------------------------------------------
void ImageManager::LoadMap(const std::string &filename)
{
	for (auto it = map.begin(); it != map.end(); ++it)
		it->second->Destroy();

	map.clear();
	XMLDoc image_list(filename);
	if (image_list.ready())
	{
		rapidxml::xml_node<char> *node = image_list.Doc()->first_node("res");
		for (auto n = node->first_node("image"); n != nullptr; n = n->next_sibling("image"))
		{
			ImageKey key;
			if (LoadImgKey(key, "name", n))
			{
				std::string path;
				LoadStr(path, "path", n, false);
				map[key] = TextureFromName(path);
			}
		}
	}
}

bool ImageManager::Init()
{
	//Load common assets
	LoadMap("core/data/common.xml");
	invalid_img = map[0];

	return true;
}

//------------------------------------------------------------------------
// Purpose: Get texture for a particular id
//------------------------------------------------------------------------
Texture* ImageManager::GetTexture(const ImageKey &id)
{
	if (map.count(id) > 0)
		return map[id];

	return invalid_img;
}

bool ImageManager::ValidTexture(const ImageKey &id)
{
	if (id != 0 && map.count(id) > 0)
		return true;

	return false;
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void ImageManager::Draw(const int &x, const int &y, const int &w, const int &h, const ImageKey &id, const SDL_Rect* clip)
{
	RenderTexture(GetTexture(id), x, y, w, h, clip);
}

void ImageManager::CircleDraw(const int &x, const int &y, const ImageKey &id, const int &start_angle, const int &end_angle, const float &radius)
{
	GLcoord2 pos(x, y);
	RenderCircularBar(GetTexture(id), start_angle, end_angle, radius, pos);
}

void ImageManager::Draw(const int &x, const int &y, const int &w, const int &h, const ImageKey &id, const float &fade, const SDL_Rect* clip)
{
	RenderTexture(GetTexture(id), x, y, w, h, fade, clip);
}

//------------------------------------------------------------------------
// Purpose: free resources
//------------------------------------------------------------------------
void ImageManager::Quit()
{
	for (auto it = map.begin(); it != map.end(); ++it)
		it->second->Destroy();

	map.clear();
}