#ifndef DRAW_UTIL_H_INCLUDED
#define DRAW_UTIL_H_INCLUDED

// For what GLUT won't do

#include <GL/gl.h>

// Must be called before using utility functions
void InitUtilFns(void);

// Draws a 1x1x1 cube, from (0, 0, 0) to (1, 1, 1)
void DrawCube(void);

// Draws a planar square
// Center at origin, on x-z plane, 1x1
void DrawSquare(void);

// Draws a planar disc (will cap cylinder nicely with a displacement)
// Center at origin, on x-z plane, diameter 1
void DrawDisc(
	unsigned segments // num of triangles to use, > 2
);

// Draws the outside of a hollow cylinder, base center at O, 1 high, diameter 1
void DrawHollowCylinder(
	unsigned segments // num of rectangular arc segments to use, > 2
);

#endif // DRAW_UTIL_H_INCLUDED
