#ifndef CAMERA_H
#define CAMERA_H

void              CameraInit(GLvector);
GLvector          CameraPosition();
GLvector2         CameraPosition2D();
GLvector2         CameraMoved();
void              CameraShake(float strength);
void              CameraTransition(GLvector2 end);
void              CameraUpdate();

#endif // CAMERA_H