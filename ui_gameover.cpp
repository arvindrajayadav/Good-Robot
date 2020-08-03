#include "master.h"
#include "ui_gameover.h"
#include "player.h"
#include "random.h"

using namespace pyrodactyl;

void GameOverMenu::Init(const char* filename)
{
	XMLDoc layout_doc(filename);
	if (layout_doc.ready())
	{
		rapidxml::xml_node<char> *node = layout_doc.Doc()->first_node("game_over");
		if (NodeValid(node))
		{
			if (NodeValid("bg", node))
				menu_bg.Load(node->first_node("bg"));

			if (NodeValid("img_bg", node))
			{
				draw_img = true;
				img_bg.Load(node->first_node("img_bg"));
			}

			if (NodeValid("audio", node))
				LoadStr(music_track, "track", node->first_node("audio"));

			if (NodeValid("reference", node))
				ref.Load(node->first_node("reference"));

			if (NodeValid("inc", node))
				inc.Load(node->first_node("inc"));

			if (NodeValid("action", node))
				me_act.Load(node->first_node("action"));

			//We're just loading the grid parameters and row value
			if (NodeValid("menu", node))
				me_trivia.Load(node->first_node("menu"));

			if (NodeValid("title", node))
				title.Load(node->first_node("title"));

			if (NodeValid("text", node))
				text.Load(node->first_node("text"));

			quote.clear();
			if (NodeValid("quotes", node))
			{
				rapidxml::xml_node<char> *qnode = node->first_node("quotes");

				for (rapidxml::xml_node<char> *n = qnode->first_node("quote"); n != nullptr; n = n->next_sibling("quote"))
				{
					Quote q;
					LoadStr(q.title, "title", n);
					LoadStr(q.text, "text", n);
					quote.push_back(q);
				}
			}

			fun_stat.clear();
			if (NodeValid("fun_stats", node))
			{
				rapidxml::xml_node<char> *qnode = node->first_node("fun_stats");

				for (rapidxml::xml_node<char> *n = qnode->first_node("stat"); n != nullptr; n = n->next_sibling("stat"))
				{
					QuoteWithNumbers q;
					LoadStr(q.title, "a", n);
					LoadStr(q.text, "b", n);
					LoadNum(q.n1, "n1", n);
					LoadNum(q.n2, "n2", n);
					fun_stat.push_back(q);
				}
			}

			//The buttons here don't matter, they'll be populated based on resolution
			me_trivia.Clear();

			//Load the list of trivia names here
			std::vector<std::string> name;

			if (NodeValid("names", node))
			{
				rapidxml::xml_node<char> *namenode = node->first_node("names");

				for (rapidxml::xml_node<char> *n = namenode->first_node(); n != nullptr; n = n->next_sibling())
				{
					std::string s;
					LoadStr(s, "name", n);
					name.push_back(s);
				}
			}

			for (int i = 0; i < 10; ++i)
			{
				Button b;
				b.Init(ref, nullptr, inc.RawPosX() * (i / me_trivia.Rows()), inc.RawPosY() * (i % me_trivia.Rows()));

				if (i < name.size())
					b.caption.text = name.at(i);

				me_trivia.element.push_back(b);
			}

			me_trivia.AssignPaths();
		}
	}
}

void GameOverMenu::Update()
{
	//Choose a random quote
	int pos = RandomVal(quote.size());

	title.text = quote.at(pos).title;
	text.text = quote.at(pos).text;

	PlayerStats *p = Player();

	me_trivia.element.at(0).desc.text = NumberToString(p->Score());
	me_trivia.element.at(1).desc.text = NumberToString(p->Trivia(TRIVIA_KILLS));
	me_trivia.element.at(2).desc.text = NumberToString(p->Trivia(TRIVIA_XP_GATHERED));
	me_trivia.element.at(3).desc.text = NumberToString(p->UpgradeCount());
	me_trivia.element.at(4).desc.text = NumberToString(p->WarrantyCount());
	me_trivia.element.at(5).desc.text = NumberToString((float)p->Trivia(TRIVIA_PLAYTIME) / 60000); 	//In minutes
	me_trivia.element.at(6).desc.text = NumberToString(p->Trivia(TRIVIA_MISSILES_DESTROYED));
	me_trivia.element.at(7).desc.text = NumberToString(p->Trivia(TRIVIA_MISSILES_EVADED));

	//The last two are random stats chosen for funny value
	pos = RandomVal(fun_stat.size());

	me_trivia.element.at(8).caption.text = fun_stat.at(pos).title;
	me_trivia.element.at(9).caption.text = fun_stat.at(pos).text;

	me_trivia.element.at(8).desc.text = NumberToString(RandomVal(fun_stat.at(pos).n1));
	me_trivia.element.at(9).desc.text = NumberToString(RandomVal(fun_stat.at(pos).n2));
}

void GameOverMenu::Draw()
{
	if (draw_img)
		img_bg.Draw();

	menu_bg.Draw();
	title.Draw();
	text.Draw();

	me_trivia.Draw();
	me_act.Draw();
}

int GameOverMenu::HandleEvents()
{
	for (int i = 0; i < me_act.element.size(); ++i)
		if (me_act.element.at(i).HandleEvents() == BUAC_LCLICK)
			return i;

	me_trivia.HandleEvents();

	return -1;
}

void GameOverMenu::SetUI()
{
	menu_bg.SetUI();
	img_bg.SetUI();

	title.SetUI();
	text.SetUI();

	ref.SetUI();
	inc.SetUI();

	me_trivia.SetUI();
	me_act.SetUI();
}