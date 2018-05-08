#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <GL/gl.h>
#include <GL/glu.h>
#ifdef __APPLE__
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif

#include "Train.h"
#include "Camera.h"
#include "Screen.h"
#include "Algebra.h"
#include "Lighting.h"
#include "DrawUtil.h"
#include "Track.h"

#define UNUSED(x) (void)(x)

#define ASIZE(arrName) ((sizeof arrName) / (sizeof arrName[0]))

#define PI 3.14159265358979323846264338327950288

// Frees allocated track on exit
static void FreeNetwork(void)
{
	TrackShared *next,
	            *track = (TrackShared *)g_initialTrackPiece.next;
	for (; track != (TrackShared *)&g_initialTrackPiece; track = next) {
		next = Track_GetNext(track);
		free(track);
	}
}

// Initialize stuff: called once from main() before main loop
static void Init(void)
{
	static const GLfloat curves[][2][2] = {
		{{3, 0}, {1, 0}},
		{{20, -10}, {1, -1}},
		{{10, -30}, {-1, -1}},
		{{-50, -40}, {-1, 0}},
		{{-60, -30}, {0, 1}},
		{{-50, -20}, {1, 0}},
		{{-10, -10}, {0, 1}},
		{{-3, 0}, {1, 0}}
	};

	glClearColor(0.7, 0.80, 0.85, 1);
	glClearAccum(0, 0, 0, 0);
	glClearDepth(1);
	glShadeModel(GL_SMOOTH);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	InitUtilFns();
	InitLighting();

	InitTrain();
	InitTrack();

	// Build track to use
	CurvedTrack *current = AllocCurvedTrack(
		curves[0][0],
		curves[0][1],
		curves[1][0],
		curves[1][1],
		NULL,
		(TrackShared *)&g_initialTrackPiece
	);
	g_initialTrackPiece.next = (TrackShared *)current;
	for (unsigned i = 1; i + 1 < ASIZE(curves); ++i) {
		current->next = (TrackShared *)AllocCurvedTrack(
			curves[i][0],
			curves[i][1],
			curves[i+1][0],
			curves[i+1][1],
			NULL,
			(TrackShared *)current
		);
		current = (CurvedTrack *)current->next;
	}
	current->next = (TrackShared *)&g_initialTrackPiece;
	g_initialTrackPiece.prev = (TrackShared *)current;

	atexit(FreeNetwork);
}

static bool antiAliasing = false;

// GLUT display callback
static void Display(void)
{
	static const GLdouble j8[8][2] = {
		{0.5625, 0.4375},
		{0.0625, 0.9375},
		{0.3125, 0.6875},
		{0.6875, 0.8125},
		{0.8125, 0.1875},
		{0.9375, 0.5625},
		{0.4375, 0.0625},
		{0.1875, 0.3125}
	};
	static const GLfloat groundColor[4] = {0.2, 0.75, 0.1, 1},
	                     blackColor[4]  = {0, 0, 0, 1},
	                     groundSize     = 1000;

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	if (antiAliasing) {
		glClear(GL_ACCUM_BUFFER_BIT);
	}

	for (unsigned jitter = 0; jitter < 8; ++jitter) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set camera position
		DrawCamera(j8[jitter][0], j8[jitter][1]);

		// Draw ground
		glMaterialfv(GL_FRONT, GL_AMBIENT, groundColor);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, groundColor);
		glMaterialfv(GL_FRONT, GL_SPECULAR, blackColor);
		glMaterialf(GL_FRONT, GL_SHININESS, 0);
		glBegin(GL_QUADS);
			glNormal3f(0, 1, 0);
			glVertex3f(groundSize/2, 0, -groundSize/2);
			glVertex3f(-groundSize/2, 0, -groundSize/2);
			glVertex3f(-groundSize/2, 0, groundSize/2);
			glVertex3f(groundSize/2, 0, groundSize/2);
		glEnd();

		// Position lights
		DrawLighting();

		// Draw train
		DrawTrain();

		// Draw track
		Track_Draw((TrackShared *)&g_initialTrackPiece);
		for (CurvedTrack *track = (CurvedTrack *)g_initialTrackPiece.next;
	     	     (TrackShared *)track != (TrackShared *)&g_initialTrackPiece;
	     	     track = (CurvedTrack *)track->next)
		{
			Track_Draw((TrackShared *)track);
		}

		// Draw track slats
		DrawSlats(0.7);

		if (!antiAliasing) {
			break;
		}
		glAccum(GL_ACCUM, 1./8);
	}

	if (antiAliasing) {
		glAccum(GL_RETURN, 1);
	}

	// Process buffered OpenGL routines and display
	glutSwapBuffers();

	// Log GL errors, if any
	GLenum error;
	if ((error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "GL error: %d\n", error);
	}
}

#define MOVE_AMOUNT 0.2

// GLUT keyboard callback
static void KeyboardCallback(unsigned char key, int x, int y)
{
	UNUSED(x); UNUSED(y);
	switch (key) {
	case ' ':
		g_cameraMode = (g_cameraMode + 1) % CameraMode_nModes;
		break;
	case 'q':
		exit(EXIT_SUCCESS);
		break;
	case 'a':
		antiAliasing = !antiAliasing;
		break;
	}
}

// GLUT special key callback
static void SpecialKeyCallback(int key, int x, int y)
{
	UNUSED(x); UNUSED(y);
	switch (key) {
	case GLUT_KEY_LEFT:
		if (g_nCarriages) {
			--g_nCarriages;
		}
		break;
	case GLUT_KEY_RIGHT:
		if (g_nCarriages < 88) {
			++g_nCarriages;
		}
		break;
	case GLUT_KEY_UP:
		g_trainSpeed += 0.004;
		if (g_trainSpeed > 0.2) {
			g_trainSpeed = 0.2;
		}
		break;
	case GLUT_KEY_DOWN:
		g_trainSpeed -= 0.004;
		if (g_trainSpeed < -0.1) {
			g_trainSpeed = -0.2;
		}
		break;
	}
}

static void ResizeCallback(int width, int height)
{
	glViewport(0, 0, width, height);
	g_screenWidth = width;
	g_screenHeight = height;
}

// GLUT animation/logic callback
static void MainStep(int value)
{
	glutTimerFunc(value, MainStep, value);
	glutPostRedisplay();
	NetworkPos_Move(&g_trainPos, g_trainSpeed);
}

int main(int argc, char *argv[])
{
	// Initialize GLUT, create window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_ACCUM | GLUT_RGB);
	g_screenWidth = 1024;
	g_screenHeight = 600;
	glutInitWindowSize(g_screenWidth, g_screenHeight);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Toy Train");

	GLenum error;
	if ((error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "GL error: %d\n", error);
	}

	// Initialize program
	ResizeCallback(g_screenWidth, g_screenHeight);
	Init();

	if ((error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "GL error: %d\n", error);
	}

	// Register GLUT callbacks, start GLUT main loop
	glutDisplayFunc(Display);
	glutKeyboardFunc(KeyboardCallback);
	glutSpecialFunc(SpecialKeyCallback);
	glutReshapeFunc(ResizeCallback);
	glutTimerFunc(1000/60, MainStep, 1000/60);
	glutMainLoop();
}
