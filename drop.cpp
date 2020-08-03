/*-----------------------------------------------------------------------------

Drop.cpp

Tracks groups of things that can be dropped, along with their probabilities. 


Good Robot
(c) 2015 Shamus Young

-----------------------------------------------------------------------------*/


#include "master.h"

#include "bodyparts.h"
#include "env.h"
#include "entity.h"
#include "loaders.h"
#include "lootpool.h"
#include "random.h"
#include "robot.h"
#include "resource.h"

#define DROP_FILE            "drop.xml"

enum DropType
{
  DROP_POOL,
  DROP_ROBOT,
  DROP_COINS,
  DROP_WEAPON,
  DROP_UNKNOWN,
};

struct DropItem
{
  string            _id;
  DropType          _type;
  int               _count;
  int               _chance;
};

struct Drop
{
  string            _name;
  vector<DropItem>  _items;
};

using namespace pyrodactyl;

static vector<Drop>         drop_table;

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

static string string_from_xml (rapidxml::xml_node<char> *node, string field)
{
  string result;

  LoadStr (result, field, node);
  return StringToLower (result);
}

DropType droptype_from_name (string name)
{
  if (!name.compare ("coins"))
    return DROP_COINS;
  if (!name.compare ("coin"))
    return DROP_COINS;
  if (EnvRobotIndexFromName (name) != ROBOT_INVALID)
    return DROP_ROBOT;
  if (EnvProjectileId (name) > 0)
    return DROP_WEAPON;
  vector<int>   temp = LootpoolFromName (name);
  if (!temp.empty ())
    return DROP_POOL;
  Console ("WARNING: Unknown item to drop. '%s'", name.c_str ());
  return DROP_UNKNOWN;

}

Drop drop_from_xml (rapidxml::xml_node<char> *node)
{
  Drop  d;
  rapidxml::xml_node<char> *item_node;

  d._name = string_from_xml (node, "name");
  item_node = node->first_node ();
  while (NodeValid (item_node)) {
    DropItem  di;
    //Get all the pertinent data regarding this item.
    di._id = string_from_xml (item_node, "name");
    di._count = StringToInt (string_from_xml (item_node, "count"));
    di._chance = StringToInt (string_from_xml (item_node, "chance"));
    //Now that we have the item, we need to know what the hell it is.
    di._type = droptype_from_name (di._id);
    d._items.push_back (di);
    item_node = item_node->next_sibling ();
  }
  return d;
}

Drop* drop_from_name (string name)
{
  name = StringToLower (name);
  for (int i = 0; i < drop_table.size (); i++) {
    if (name.compare (drop_table[i]._name.c_str ()) == 0)
      return &drop_table[i];
  }
  return NULL;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void DropInit ()
{
  XMLDoc drop_file (ResourceLocation (DROP_FILE, RESOURCE_DATA));

  if (!drop_file.ready ())
    return;
  rapidxml::xml_node<char> *top_node = drop_file.Doc ()->first_node ("drops");
  if (!NodeValid (top_node))
    return;
  rapidxml::xml_node<char> *node = top_node->first_node ();
  while (NodeValid (node)) {
    drop_table.push_back (drop_from_xml (node));
    node = node->next_sibling ();
  }
}

void DropLoot (string name, GLvector2 pos)
{
  Drop*       d = drop_from_name (name);
  DropItem*   item;

  if (d == NULL)
    return;
  for (int i = 0; i < d->_items.size (); i++) {
    item = &d->_items[i];
    //a value of 1 or lower always passes...
    if (item->_chance > 1) {
      if (!RandomRoll (item->_chance))
        continue; //Nope, don't drop this, whatever it is.
    }
    if (item->_type == DROP_COINS)
      EntityXpAdd (pos, item->_count);
    if (item->_type == DROP_ROBOT) {
      RobotType id = EnvRobotIndexFromName (item->_id);
      for (int r = 0; r < item->_count; r++) {
        Robot b;
        b.Init (pos, id);
        EntityRobotAdd (b);
      }
    }
    if (item->_type == DROP_POOL) {
      vector<int>   list = LootpoolFromName (item->_id);
      int   roll = RandomVal (list.size ());
      fxPickup*   p = new fxPickup;
      p->InitGun (pos, list[roll]);
      EntityFxAdd (p);
    }
    if (item->_type == DROP_WEAPON) {
      int id = EnvProjectileId (item->_id);
      fxPickup*   p = new fxPickup;
      p->InitGun (pos, id);
      EntityFxAdd (p);
    }
  }



}
