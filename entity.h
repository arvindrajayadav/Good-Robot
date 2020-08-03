#ifndef ENTITY_H
#define ENTITY_H

#include "fx.h"

#define MAX_PROJECTILES     1000

void                EntityClear();
void                EntityDeviceAdd (class fxDevice* d);
int                 EntityDeviceCount ();
void                EntityDeviceRender (bool occluded);

fxDevice*           EntityDeviceFromId (int id);

void                EntityFxAdd(class fx* item);
class fxProjectile* EntityProjectileList ();
void                EntityProjectileAdd (class fxProjectile p);
void                EntityProjectileFire (enum fxOwner owner, const class Projectile* p_info, GLvector2 origin, GLvector2 vector, int damage_level=0);
void                EntityRenderRobots(bool hidden);
void                EntityRenderFx();
class Robot*        EntityRobot (int index);
void                EntityRobotAdd (class Robot b);
int                 EntityRobotCount ();
int                 EntityRobotsActive ();
int									EntityRobotsDead ();
Robot*              EntityRobotFromId (int id);
void                EntityUpdate();
void                EntityXpAdd(GLvector2 position, int xp);

#endif // ENTITY_H
