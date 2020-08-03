//=============================================================================
// Author:   Arvind
// Purpose:  Contains the image manager class - used to manage in-game assets
//=============================================================================
#pragma once

#include "common_header.h"
#include "loaders.h"
#include "texture.h"
#include "render.h"
#include "XMLDoc.h"

//Since we use unsigned int as a key for images, our LoadImgKey function is LoadNum
#define LoadImgKey LoadNum

namespace pyrodactyl
{
	//We use this object as the key for all image assets
	typedef unsigned int ImageKey;

	//We store images here
	typedef std::unordered_map<ImageKey, Texture*> TextureMap;

	class ImageManager
	{
		//Assets are stored in images
		//Common is stuff used everywhere - this is only loaded once
		TextureMap map;

		//The default image for all invalid image names
		Texture* invalid_img;

	public:
		ImageManager(){}
		~ImageManager(){ Quit(); }

		void Quit();

		bool Init();
		void LoadPaths();

		//image related stuff

		//Load all images specified in an XML file in a map
		void LoadMap(const std::string &filename);

		void FreeTexture(const ImageKey &id) { map[id]->Destroy(); }
		Texture* GetTexture(const ImageKey &id);
		bool ValidTexture(const ImageKey &id);

		//Draw a texture
		void Draw(const int &x, const int &y, const int &w, const int &h, const ImageKey &id, const SDL_Rect* clip = nullptr);

		//Draw a circle
		void CircleDraw(const int &x, const int &y, const ImageKey &id, const int &start_angle, const int &end_angle, const float &radius);

		//Draw a texture with changing alpha
		void Draw(const int &x, const int &y, const int &w, const int &h, const ImageKey &id, const float &fade, const SDL_Rect* clip = nullptr);
	};

	extern ImageManager gImageManager;
}