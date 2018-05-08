#include <math.h>
#include <assert.h>

#include "DrawUtil.h"

#define PI 3.14159265358979323846264338327950288

#define RADS (PI/180)

static GLuint cubeDl   = 0,
              squareDl = 0;

void InitUtilFns(void)
{
	cubeDl = glGenLists(1);
	assert(cubeDl);
	glNewList(cubeDl, GL_COMPILE);
		glBegin(GL_QUADS);
			glNormal3f(0, 0, -1);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 1, 0);
			glVertex3f(1, 1, 0);
			glVertex3f(1, 0, 0);

			glNormal3f(1, 0, 0);
			glVertex3f(1, 0, 0);
			glVertex3f(1, 1, 0);
			glVertex3f(1, 1, 1);
			glVertex3f(1, 0, 1);

			glNormal3f(0, 0, 1);
			glVertex3f(1, 0, 1);
			glVertex3f(1, 1, 1);
			glVertex3f(0, 1, 1);
			glVertex3f(0, 0, 1);

			glNormal3f(-1, 0, 0);
			glVertex3f(0, 0, 1);
			glVertex3f(0, 1, 1);
			glVertex3f(0, 1, 0);
			glVertex3f(0, 0, 0);

			glNormal3f(0, 1, 0);
			glVertex3f(1, 1, 1);
			glVertex3f(1, 1, 0);
			glVertex3f(0, 1, 0);
			glVertex3f(0, 1, 1);

			glNormal3f(0, -1, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(1, 0, 0);
			glVertex3f(1, 0, 1);
			glVertex3f(0, 0, 1);
		glEnd();
	glEndList();

	squareDl = glGenLists(1);
	assert(squareDl);
	glNewList(squareDl, GL_COMPILE);
		glBegin(GL_QUADS);
			glNormal3f(0, 1, 0);
			glVertex3f(-0.5, 0, -0.5);
			glVertex3f(-0.5, 0, 0.5);
			glVertex3f(0.5, 0, 0.5);
			glVertex3f(0.5, 0, -0.5);
		glEnd();
	glEndList();
}

void DrawCube(void)
{
	glCallList(cubeDl);
}

void DrawSquare(void)
{
	glCallList(squareDl);
}

void DrawDisc(unsigned segments)
{
	glBegin(GL_TRIANGLE_FAN);
		glNormal3f(0, 1, 0);
		glVertex3f(0, 0, 0);
		for (unsigned i = 0; i <= segments; ++i) {
			GLfloat radAngle = RADS*360*i/segments;
			GLfloat cosine = -0.5*cosf(radAngle), sine = 0.5*sinf(radAngle);
			glNormal3f(0, 1, 0);
			glVertex3f(cosine, 0, sine);
		}
	glEnd();
}

void DrawHollowCylinder(unsigned segments)
{
	glBegin(GL_TRIANGLE_STRIP);
		for (unsigned i = 0; i <= segments; ++i) {
			GLfloat radAngle = RADS*360*i/segments;
			GLfloat cosine = 0.5*cosf(radAngle), sine = 0.5*sinf(radAngle);
			glNormal3f(cosine, 0, sine);
			glVertex3f(cosine, 0, sine);
			glNormal3f(cosine, 0, sine);
			glVertex3f(cosine, 1, sine);
		}
	glEnd();
}
