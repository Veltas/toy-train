#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include <GL/gl.h>

enum {
	CameraMode_birdseye,
	CameraMode_train,
	CameraMode_trainSide,
	CameraMode_nModes
};

extern int g_cameraMode;

// Positions the camera (doesn't actually draw anything)
void DrawCamera(GLdouble pixdx, GLdouble pixdy);

#endif // CAMERA_H_INCLUDED
