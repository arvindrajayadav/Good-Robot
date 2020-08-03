/*-----------------------------------------------------------------------------

Lootpool.cpp

Tracks lists of weapons that can be dropped. (As in: A robot dies, and drops a 
weapon.)

Good Robot
(c) 2015 Shamus Young

-----------------------------------------------------------------------------*/

#include "master.h"

#include "ini.h"
#include "env.h"
#include "resource.h"

#define LOOT_FILE            "loot.ini"

class Pool
{
  string          _name;
  vector<int>     _list;

public:
  void            Init (string name, string list);
  int             Drop ();
  const char*     Name () { return _name.c_str (); }
  vector<int>     List () { return _list;  }
};

static vector<Pool>       pool_weapons;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void Pool::Init (string name, string list)
{
  vector<string>  weapon_names;
  vector<int>     result;

  weapon_names = StringSplit (list, " ,");
  for (unsigned i = 0; i < weapon_names.size (); i++) {
    result.push_back (EnvProjectileId (weapon_names[i]));
  }
  if (result.empty ())
    result.push_back (0);
  _name = StringToLower (name);
  _list.clear ();
  _list = result;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static Pool* pool_from_name (string name)
{
  name = StringToLower (name);
  for (int i = 0; i < pool_weapons.size (); i++) {
    if (!name.compare (pool_weapons[i].Name ()))
      return &pool_weapons[i];
  }
  //"Shouldn't happen".
  return NULL;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

#define SECTION  "Drops"

void LootpoolInit ()
{
  iniFile         ini;
  unsigned        keys;
  Pool            p;

  ini.Open (ResourceLocation (LOOT_FILE, RESOURCE_DATA));
  keys = ini.SectionKeys (SECTION);
  for (unsigned i = 0; i < keys; i++) {
    string  key = ini.SectionKey (SECTION, i);
    string  list = ini.StringGet (SECTION, key);

    p.Init (StringToLower (key), list);
    pool_weapons.push_back (p);
  }
}

vector<int> LootpoolFromName (string name)
{
  Pool*   p;

  p = pool_from_name (name);
  if (p == NULL) {
    vector<int>   result;
    return result;
  }
  return p->List ();
}