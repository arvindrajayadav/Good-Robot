#include "master.h"
#include "ProgressBar.h"

using namespace pyrodactyl;

void ProgressBar::Draw(int x, int y, int h, float pixels_per_unit, int val, bool end_incl_in_val)
{
	//Don't draw the bar if value is 0
	if (val > 0)
	{
		//We have to draw val * pixels_per_unit of health bar in total
		int total_w = static_cast<int>(val * pixels_per_unit);

		//First draw the start image and increment the value of x by its width
		if (start.img != 0)
		{
			gImageManager.Draw(x, y, start.w, h, start.img);
			x += start.w;

			//This is the remaining amount of pixels to draw
			total_w -= start.w;
		}

		//Decrease x by 1 to prevent a 1px gap for some values of pixels_per_unit
		//x--;

		if (end.img != 0)
		{
			if (end_incl_in_val)
				total_w -= end.w;
			else
				total_w += offset_end.x;
		}
		else
			total_w--; //Decrease total_w by 1 if we know we're not drawing the end image, to prevent a 1px gap for some values of pixels_per_unit

		//The middle image is repeated until the correct amount of health is reached
		if (total_w > 0 && mid.w > 0 && mid.img != 0)
		{
			//The number of times the full image is drawn
			int full = total_w / mid.w;

			for (int i = 0; i < full; ++i)
			{
				gImageManager.Draw(x, y, mid.w, h, mid.img);
				x += mid.w;
			}

			//Draw the clipped portion, if any
			unsigned short partial = total_w %  mid.w;
			if (partial != 0)
			{
				SDL_Rect clip;
				clip.x = 0; clip.y = 0; clip.w = partial; clip.h = mid.h;
				gImageManager.Draw(x, y, partial, h, mid.img, &clip);
				x += partial;
			}

			//Draw the end image
			if (end.img != 0)
			{
				//Decrease x by 1 to prevent a 1px gap for some values of pixels_per_unit
				//x--;

				gImageManager.Draw(x, y + offset_end.y, end.w, h, end.img);
			}
		}
	}
}