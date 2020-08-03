#ifndef COLLISION_H
#define COLLISION_H

bool      Collision(GLvector2 point, GLvector2* normal, float* depth);
bool      Collision(GLvector2 point);
bool      Collision(GLvector2 position, float radius);
bool      Collision(GLvector2 position, GLvector2* normal, float* depth, float radius);
float     CollisionCeiling(GLvector2 point);
float     CollisionFloor(GLvector2 point);
bool      CollisionLine(GLcoord2 cell, vector<Line2D>& lines);
bool      CollisionLos(GLvector2 start, GLvector2 end, float interval);
void      CollisionRender();
GLvector2 CollisionSlide(GLvector2 wall, GLvector2 movement);

#endif // COLLISION_H