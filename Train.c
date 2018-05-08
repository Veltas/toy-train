#include <assert.h>
#include <GL/gl.h>
#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif
#include "DrawUtil.h"
#include "Track.h"
#include "Algebra.h"

#include "Train.h"

static GLuint trainDl,
              carriageDl;

unsigned g_nCarriages = 5;

GLfloat g_trainSpeed = 0.01;

NetworkPos g_trainPos = {(TrackShared *)&g_initialTrackPiece, 1.5};

static GLfloat whiteColor[4] = {1, 1, 1, 1},
               grayColor[4]  = {0.5, 0.5, 0.5, 1},
               redColor[4]   = {1, 0.1, 0.1, 1},
               darkColor[4]  = {0.2, 0.18, 0.14, 1},
               metalColor[4] = {0.7, 0.7, 0.75, 1},
               blackColor[4] = {0, 0, 0, 1};

static void DrawWheel(void)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, metalColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, metalColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 50);
	glPushMatrix();
		glRotatef(-90, 1, 0, 0);
		glScalef(0.3, 0.02, 0.3);
		DrawHollowCylinder(24);
		glTranslatef(0, 1, 0);
		DrawDisc(24);
	glPopMatrix();
	glPushMatrix();
		glRotatef(90, 1, 0, 0);
		glPushMatrix();
			glScalef(0.3, 1, 0.3);
			glRotatef(180, 0, 1, 0);
			DrawDisc(24);
		glPopMatrix();
		glScalef(0.25, 0.03, 0.25);
		DrawHollowCylinder(16);
		glTranslatef(0, 1, 0);
		DrawDisc(16);
	glPopMatrix();
}

static void DrawSpoke(void)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, metalColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, metalColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 50);
	glPushMatrix();
		glRotatef(90, 1, 0, 0);
		glScalef(0.05, 1-0.08, 0.05);
		glTranslatef(0, -0.5, 0);
		DrawHollowCylinder(12);
	glPopMatrix();
}

void InitTrain(void)
{
	// Compile train model
	trainDl = glGenLists(1);
	assert(trainDl);
	glNewList(trainDl, GL_COMPILE);
		// Draw locomotive carriage
		glMaterialfv(GL_FRONT, GL_AMBIENT, redColor);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, redColor);
		glMaterialfv(GL_FRONT, GL_SPECULAR, grayColor);
		glMaterialf(GL_FRONT, GL_SHININESS, 50);
		glPushMatrix();
			glTranslatef(-1, 0.45, 0);
			glScalef(0.5, 1, 1);
			glTranslatef(0, 0, -0.5);
			DrawCube();
		glPopMatrix();

		// Draw locomotive tank
		glPushMatrix();
			glTranslatef(-0.5, 0.45 + 1./3, 0);
			glRotatef(-90, 0, 0, 1);
			glScalef(2./3, 1.5, 2./3);
			DrawHollowCylinder(32);
			glTranslatef(0, 1, 0);
			DrawDisc(32);
		glPopMatrix();

		// Draw tank chimney
		glPushMatrix();
			glTranslatef(0.5, 0.45 + 1./3, 0);
			glScalef(0.3, 0.7, 0.3);
			DrawHollowCylinder(32);
			glTranslatef(0, 1, 0);
			DrawDisc(32);
		glPopMatrix();

		// Draw undercarriage
		glMaterialfv(GL_FRONT, GL_AMBIENT, darkColor);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, darkColor);
		glMaterialfv(GL_FRONT, GL_SPECULAR, blackColor);
		glMaterialf(GL_FRONT, GL_SHININESS, 0);
		glPushMatrix();
			glTranslatef(-1, 0.15, 0);
			glScalef(2, 0.3, 0.7);
			glTranslatef(0, 0, -0.5);
			DrawCube();
		glPopMatrix();

		// Draw spokes
		glPushMatrix();
			glTranslatef(-0.5, 0.225, 0);
			DrawSpoke();
			glTranslatef(1, 0, 0);
			DrawSpoke();
		glPopMatrix();

		// Draw wheels
		glPushMatrix();
			glTranslatef(-0.5, 0.225, 0.48);
			DrawWheel();
			glPushMatrix();
				glTranslatef(0, 0, -0.96);
				glRotatef(180, 0, 1, 0);
				DrawWheel();
				glTranslatef(-1, 0, 0);
				DrawWheel();
			glPopMatrix();
			glTranslatef(1, 0, 0);
			DrawWheel();
		glPopMatrix();
	glEndList();

	// Compile carriage model
	carriageDl = glGenLists(1);
	assert(carriageDl);
	glNewList(carriageDl, GL_COMPILE);
		// Draw top carriage
		glMaterialfv(GL_FRONT, GL_AMBIENT, redColor);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, redColor);
		glMaterialfv(GL_FRONT, GL_SPECULAR, grayColor);
		glMaterialf(GL_FRONT, GL_SHININESS, 50);
		glPushMatrix();
			glTranslatef(-1, 0.45, 0);
			glScalef(2, 1, 1);
			glTranslatef(0, 0, -0.5);
			DrawCube();
		glPopMatrix();

		// Draw undercarriage
		glMaterialfv(GL_FRONT, GL_AMBIENT, darkColor);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, darkColor);
		glMaterialfv(GL_FRONT, GL_SPECULAR, blackColor);
		glMaterialf(GL_FRONT, GL_SHININESS, 0);
		glPushMatrix();
			glTranslatef(-1, 0.15, 0);
			glScalef(2, 0.3, 0.7);
			glTranslatef(0, 0, -0.5);
			DrawCube();
		glPopMatrix();

		// Draw hook
		glPushMatrix();
			glTranslatef(0.5, 0.45, 0);
			glScalef(1, 1, 1);
			glBegin(GL_TRIANGLES);
				glNormal3f(0, 1, 0);
				glVertex3f(0, 0, -0.25);
				glVertex3f(0, 0, 0.25);
				glVertex3f(1, 0, 0);
			glEnd();
		glPopMatrix();

		// Draw spokes
		glPushMatrix();
			glTranslatef(-0.5, 0.225, 0);
			DrawSpoke();
			glTranslatef(1, 0, 0);
			DrawSpoke();
		glPopMatrix();

		// Draw wheels
		glPushMatrix();
			glTranslatef(-0.5, 0.225, 0.48);
			DrawWheel();
			glPushMatrix();
				glTranslatef(0, 0, -0.96);
				glRotatef(180, 0, 1, 0);
				DrawWheel();
				glTranslatef(-1, 0, 0);
				DrawWheel();
			glPopMatrix();
			glTranslatef(1, 0, 0);
			DrawWheel();
		glPopMatrix();
	glEndList();
}

void DrawTrain(void)
{
	// Draw locomotive
	GLfloat wheelPos[2][3];
	NetworkPos wheelNp = g_trainPos;
	NetworkPos_Move(&wheelNp, -0.5);
	Track_GetCoords(wheelNp.track, wheelPos[0], wheelNp.pos);
	NetworkPos_Move(&wheelNp, 1);
	Track_GetCoords(wheelNp.track, wheelPos[1], wheelNp.pos);
	GLfloat forward[3], pos[3];
	Normalize3(forward, Saxpy3(forward, wheelPos[1], -1, wheelPos[0]));
	Saxpy3(pos, wheelPos[0], 0.5, forward);
	glPushMatrix();
		// Train location
		glTranslatef(pos[0], pos[1], pos[2]);
		// Train orientation
		Normalize3(forward, forward);
		GLfloat angle = Angle3((GLfloat [3]){1}, forward);
		if (Cross3((GLfloat [3]){0}, (GLfloat [3]){1}, forward)[1] < 0) {
			angle *= -1;
		}
		glRotatef(angle, 0, 1, 0);
		// Draw train
		glCallList(trainDl);
	glPopMatrix();

	// Draw carriages
	for (unsigned i = 0; i < g_nCarriages; ++i) {
		NetworkPos_Move(&wheelNp, -3.3);
		Track_GetCoords(wheelNp.track, wheelPos[0], wheelNp.pos);
		NetworkPos_Move(&wheelNp, 1);
		Track_GetCoords(wheelNp.track, wheelPos[1], wheelNp.pos);
		Normalize3(forward, Saxpy3(forward, wheelPos[1], -1, wheelPos[0]));
		Saxpy3(pos, wheelPos[0], 0.5, forward);
		glPushMatrix();
			// Carriage location
			glTranslatef(pos[0], pos[1], pos[2]);
			// Carriage orientation
			Normalize3(forward, forward);
			angle = Angle3((GLfloat [3]){1}, forward);
			if (Cross3((GLfloat [3]){0}, (GLfloat [3]){1}, forward)[1] < 0) {
				angle *= -1;
			}
			glRotatef(angle, 0, 1, 0);
			// Draw carriage
			glCallList(carriageDl);
		glPopMatrix();
	}
}
