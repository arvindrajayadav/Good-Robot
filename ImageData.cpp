#include "master.h"
#include "ImageData.h"

using namespace pyrodactyl;

void ImageData::Load(rapidxml::xml_node<char> *node, pyroRect *parent, const bool &echo)
{
	LoadImgKey(key, "img", node, echo);
	Element::Load(node, parent, echo);
}

void ImageData::Draw(const SDL_Rect *clip)
{
	gImageManager.Draw(x, y, w, h, key, clip);
}

void ImageData::CircleDraw(const int &start_angle, const int &end_angle, const float &radius)
{
	gImageManager.CircleDraw(x, y, key, start_angle, end_angle, radius);
}

void ImageData::FadeDraw(const float &fade)
{
	gImageManager.Draw(x, y, w, h, key, nullptr);
}