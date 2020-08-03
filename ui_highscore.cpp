#include "master.h"
#include "ui_highscore.h"
#include "steam_data.h"

using namespace pyrodactyl;

//-----------------------------------------------------------------------------
// Purpose: Add score to both personal history and steam
//-----------------------------------------------------------------------------
void HighScoreMenu::AddScore(const HighScoreData &h)
{
	local_leaderboard.AddScore(h);
	steam_leaderboard.AddScore(h.score, h.kills);
}

//-----------------------------------------------------------------------------
// Purpose: Load interface layout
//-----------------------------------------------------------------------------
void HighScoreMenu::Init()
{
	XMLDoc layout_doc("core/data/ui_score.xml");
	if (layout_doc.ready())
	{
		rapidxml::xml_node<char> *node = layout_doc.Doc()->first_node("score");
		if (NodeValid(node))
		{
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			if (NodeValid("back", node))
				back.Load(node->first_node("back"));

			if (NodeValid("switcher", node))
				switcher.Load(node->first_node("switcher"));

			if (NodeValid("local_leaderboard", node))
				local_leaderboard.Load(node->first_node("local_leaderboard"));

			if (NodeValid("steam_leaderboard", node))
				steam_leaderboard.Load(node->first_node("steam_leaderboard"));
		}
	}
}

bool HighScoreMenu::HandleEvents(const bool esc_key_pressed)
{
	if (switcher.HandleEvents())
		state = static_cast<StateType>(switcher.CurV().val);

	switch (state)
	{
	case STATE_LOCAL:
		local_leaderboard.HandleEvents();
		break;

	case STATE_GLOBAL:
		steam_leaderboard.HandleEvents();
		break;

	default: break;
	}

	return (back.HandleEvents() == BUAC_LCLICK || esc_key_pressed);
}

void HighScoreMenu::Draw()
{
	bg.Draw();
	switcher.Draw();

	switch (state)
	{
	case STATE_LOCAL:
		local_leaderboard.Draw();
		break;

	case STATE_GLOBAL:
		steam_leaderboard.Draw();
		break;

	default: break;
	}

	back.Draw();
}

void HighScoreMenu::SetUI()
{
	bg.SetUI();
	back.SetUI();
	switcher.SetUI();

	local_leaderboard.SetUI();
	steam_leaderboard.SetUI();
}