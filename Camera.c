#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "Screen.h"
#include "Train.h"
#include "Algebra.h"
#include "Track.h"

#include "Camera.h"

#define PI 3.14159265358979323846264338327950288

int g_cameraMode = 0;

// From OpenGL red book
static void accFrustum(
	GLdouble left, GLdouble right, GLdouble bottom, GLdouble top,
	GLdouble near, GLdouble far,
	GLdouble pixdx, GLdouble pixdy,
	GLdouble eyedx, GLdouble eyedy,
	GLdouble focus)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble xwsize = right - left,
	         ywsize = top - bottom,
	         dx     = -(pixdx*xwsize/viewport[2] + eyedx*near/focus),
	         dy     = -(pixdy*ywsize/viewport[3] + eyedy*near/focus);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(left + dx, right + dx, bottom + dy, top + dy, near, far);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-eyedx, -eyedy, 0);
}

// From OpenGL red book
static void accPerspective(
	GLdouble fovy, GLdouble aspect,
	GLdouble near, GLdouble far,
	GLdouble pixdx, GLdouble pixdy,
	GLdouble eyedx, GLdouble eyedy,
	GLdouble focus)
{
	GLdouble fov2   = fovy*PI / (180 * 2),
	         top    = sin(fov2) * near / cos(fov2),
	         bottom = -top,
	         right  = top * aspect,
	         left   = -right;

	accFrustum(
		left, right, bottom, top,
		near, far,
		pixdx, pixdy,
		eyedx, eyedy,
		focus
	);
}

void DrawCamera(GLdouble pixdx, GLdouble pixdy)
{
	GLdouble fov         = 45,
	         aspectRatio = (GLdouble)g_screenWidth/g_screenHeight;
	if (aspectRatio < 16./9 && aspectRatio > 5./4) {
		fov *= 16./9 / aspectRatio;
	} else if (aspectRatio <= 5./4) {
		fov *= 16./9 / (5./4);
	}
	accPerspective(
		fov,
		aspectRatio,
		0.05, 1000,
		pixdx, pixdy,
		0, 0,
		1
	);

	switch (g_cameraMode) {
	case CameraMode_birdseye:
		gluLookAt(-20, 60, -22, -20, 0, -22, cosf(1), 0, -sinf(1));
		break;
	case CameraMode_train:
		{
			GLfloat wheelPos[2][3];
			NetworkPos wheelNp = g_trainPos;
			NetworkPos_Move(&wheelNp, -0.5);
			Track_GetCoords(wheelNp.track, wheelPos[0], wheelNp.pos);
			NetworkPos_Move(&wheelNp, 1);
			Track_GetCoords(wheelNp.track, wheelPos[1], wheelNp.pos);
			GLfloat forward[3], pos[3];
			Normalize3(forward, Saxpy3(forward, wheelPos[1], -1, wheelPos[0]));
			Saxpy3(pos, wheelPos[0], 0.5, forward);
			GLfloat angle = Angle3((GLfloat [3]){0, 0, 1}, forward);
			if (Cross3((GLfloat [3]){0}, (GLfloat [3]){0, 0, 1}, forward)[1] > 0) {
				angle *= -1;
			}
			glTranslatef(0.35, -1.3, -0.5);
			glRotatef(180 + angle, 0, 1, 0);
			gluLookAt(
				pos[0], pos[1], pos[2],
				pos[0], pos[1], pos[2] - 100,
				0, 1, 0
			);
		}
		break;
	case CameraMode_trainSide:
		{
			GLfloat wheelPos[2][3];
			NetworkPos wheelNp = g_trainPos;
			NetworkPos_Move(&wheelNp, -0.5);
			Track_GetCoords(wheelNp.track, wheelPos[0], wheelNp.pos);
			NetworkPos_Move(&wheelNp, 1);
			Track_GetCoords(wheelNp.track, wheelPos[1], wheelNp.pos);
			GLfloat forward[3], pos[3];
			Normalize3(forward, Saxpy3(forward, wheelPos[1], -1, wheelPos[0]));
			Saxpy3(pos, wheelPos[0], 0.5, forward);
			GLfloat angle = Angle3((GLfloat [3]){0, 0, 1}, forward);
			if (Cross3((GLfloat [3]){0}, (GLfloat [3]){0, 0, 1}, forward)[1] > 0) {
				angle *= -1;
			}
			glTranslatef(-0.3, -1, -4);
			glRotatef(20, 1, 0, 0);
			glRotatef(60 + angle, 0, 1, 0);
			gluLookAt(
				pos[0], pos[1], pos[2],
				pos[0], pos[1], pos[2] - 100,
				0, 1, 0
			);
		}
		break;
	}
}
