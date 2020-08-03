#pragma once

#include "common_header.h"
#include "RectangleProgressBar.h"
#include "CircleProgressBar.h"
#include "WorldTitleData.h"
#include "button.h"
#include "player.h"

namespace pyrodactyl
{
	class HUD
	{
		//Button image _b = when multiplier is 0, _h when multiplier is 1-limit, _s when multiplier is >limit
		//Caption = score value, desc = the text "score", tooltip = multiplier
		Button score;

		//The value at which multiplier changes color
		int limit;

		//Button image _b = normal, _h when money is increased, _s when money is decreased
		//Caption = money value, desc = the text "money"
		Button money;

		//Player health bar
		CircleProgressBar health_bar;

		//Boss health bar
		RectangleProgressBar boss_bar;

		//If these change, we recompile the HUD
		PlayerStats last_stats;

		//Warranty status shown here
		ImageData warranty;

		//The place where primary and secondary weapon icons are drawn
		Element weapon_primary, weapon_secondary;

		//If we're in a boss battle, set this to true
		bool boss_exists;

		//Name of the current boss - used to detect health bar changes for multiple
		//bosses in the same level
		std::string boss_name;

		//The info for drawing the compass
		class Compass : public Element
		{
		public:
			//Compass background
			ImageData bg;

			//The exit symbol
			ImageData sym_exit;

			GLrgba color;
			float size;

			Compass() : color(255, 255, 255, 255) { size = 0.0f; }

			void Load(rapidxml::xml_node<char> *node)
			{
				Element::Load(node);
				LoadNum(size, "size", node);
				LoadColor(color, node);

				if (NodeValid("bg", node))
					bg.Load(node->first_node("bg"));

				if (NodeValid("exit", node))
					sym_exit.Load(node->first_node("exit"));
			}

			void Draw(const float angle)
			{
				bg.Draw();

				GLvector2 pos((float)x, (float)y);
				glBindTexture(GL_TEXTURE_2D, SpriteMapTexture());
				RenderQuadNow(pos, SPRITE_ARROW, color, size, angle, 0.0f, false);
			}

			void DrawExit()
			{
				bg.Draw();
				sym_exit.Draw();
			}

			void SetUI()
			{
				Element::SetUI();
				bg.SetUI();
				sym_exit.SetUI();
			}
		} compass;

		//The world title is drawn using this
		WorldTitleData world_title;

		//The zone clear info is drawn using this
		WorldTitleData zone_clear;

	public:
		HUD() { limit = 4; boss_exists = false; }

		void Load(const std::string &filename);
		void Draw();
		void SetUI();
	};
}