/*-----------------------------------------------------------------------------

  Message.cpp

  Good Robot
  (c) 2013 Shamus Young

  Reads in a text file that provides (eventually) all user-facing text.
  This is done to make it less painful to add multi-language / unicode
  support in the future, and also to avoid having gameplay text showing up
  as hardcoded values all over the place.

  -----------------------------------------------------------------------------*/

#include "master.h"

#include <vector>
#include <algorithm>
#include <map>

#include "ini.h"
#include "resource.h"

#define MESSAGE_FILE     "message.ini"

struct msg
{
	string    key;
	string    value;
};

static vector<msg>      msg_map;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static string msg_lookup(string key)
{
	for (unsigned i = 0; i < msg_map.size(); i++) {
		if (msg_map[i].key == key)
			return msg_map[i].value;
	}
	return string("Key not found: \"") + key + string("\"");
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void MessageInit()
{
	iniFile     ini;
	string      section;
	msg         entry;

	section = "english";
	ini.Open(ResourceLocation(MESSAGE_FILE, RESOURCE_DATA));
	for (unsigned i = 0; i < ini.SectionKeys(section); i++) {
		entry.key = ini.SectionKey(section, i);
		entry.value = ini.SectionValue(section, i);
		msg_map.push_back(entry);
	}
	Console("MessageInit: %d entries loaded from '%s'", msg_map.size(), MESSAGE_FILE);
}

string Message(string id)
{
	return msg_lookup(id);
}