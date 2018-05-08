#include <GL/gl.h>

#include "Lighting.h"

void InitLighting(void)
{
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (GLfloat [4]){0.5, 0.5, 0.45, 1});

	glLightfv(GL_LIGHT0, GL_AMBIENT, (GLfloat [4]){0, 0, 0, 1});
	glLightfv(GL_LIGHT0, GL_DIFFUSE, (GLfloat [4]){0.5, 0.5, 0.5, 1});
	glLightfv(GL_LIGHT0, GL_SPECULAR, (GLfloat [4]){1, 1, 1, 1});

	glEnable(GL_LIGHT0);
}

void DrawLighting(void)
{
	glLightfv(GL_LIGHT0, GL_POSITION, (GLfloat [4]){-5, 300, 200, 1});
}
