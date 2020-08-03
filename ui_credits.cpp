#include "master.h"
#include "ui_credits.h"

using namespace pyrodactyl;

//--------------------------------------------------------------------
//Opening a website in the default web browser in different OSes
//--------------------------------------------------------------------

#ifdef __WIN32__
#include <windows.h>
#include <ShellAPI.h>
#pragma comment(lib,"ws2_32.lib")
#endif

#ifdef __APPLE__
#include <CoreFoundation/CFBundle.h>
#include <ApplicationServices/ApplicationServices.h>
#endif

void OpenURL(const std::string &url_str)
{
#ifdef __WIN32__
	ShellExecuteA(NULL, "open", url_str.c_str(), NULL, NULL, SW_SHOW);
#endif

#ifdef __APPLE__
	CFURLRef url = CFURLCreateWithBytes(
		NULL,                        // allocator
		(UInt8*)url_str.c_str(),     // URLBytes
		url_str.length(),            // length
		kCFStringEncodingASCII,      // encoding
		NULL                         // baseURL
		);
	LSOpenCFURLRef(url, 0);
	CFRelease(url);
#endif

#ifdef __GNUC__
	std::string command = "xdg-open " + url_str;
	system(command.c_str());
#endif
}

//------------------------
//Credits screen
//------------------------

void CreditScreen::Init()
{
	XMLDoc conf("core/data/ui_credits.xml");
	if (conf.ready())
	{
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("credits");

		if (NodeValid("bg", node))
			bg.Load(node->first_node("bg"));

		if (NodeValid("h", node))
			heading.Load(node->first_node("h"));

		if (NodeValid("p", node))
			paragraph.Load(node->first_node("p"));

		if (NodeValid("logo", node))
			logo.Load(node->first_node("logo"));

		if (NodeValid("menu", node))
			menu.Load(node->first_node("menu"));

		if (NodeValid("start", node))
			start.Load(node->first_node("start"));

		if (NodeValid("text", node))
		{
			rapidxml::xml_node<char> *tnode = node->first_node("text");
			for (rapidxml::xml_node<char> *n = tnode->first_node(); n != NULL; n = n->next_sibling())
			{
				CreditText t;
				t.text = n->value();
				t.heading = (n->name()[0] == 'h');
				list.push_back(t);
			}
		}
	}
}

bool CreditScreen::HandleEvents(bool esc_key_pressed)
{
	int choice = menu.HandleEvents();
	switch (choice)
	{
	case 0: OpenURL("http://pyrodactyl.com"); break;
	case 1: OpenURL("https://www.twitter.com/pyrodactylgames"); break;
	case 2: OpenURL("http://www.shamusyoung.com/twentysidedtale/"); break;
	case 3: return true;
	default: break;
	}

	return esc_key_pressed;
}

void CreditScreen::Draw()
{
	bg.Draw();
	logo.Draw();

	Element cur = start;
	for (auto i = list.begin(); i != list.end(); ++i)
	{
		if (i->heading)
		{
			cur.y += heading.inc.y;
			gTextManager.Draw(cur.x, cur.y, i->text, heading.color, heading.font, heading.text_align);
		}
		else
		{
			cur.y += paragraph.inc.y;
			gTextManager.Draw(cur.x, cur.y, i->text, paragraph.color, paragraph.font, paragraph.text_align);
		}
	}

	menu.Draw();
}

void CreditScreen::SetUI()
{
	bg.SetUI();
	menu.SetUI();
	logo.SetUI();

	start.SetUI();
	heading.SetUI();
	paragraph.SetUI();
}