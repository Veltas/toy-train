#ifndef ALGEBRA_H_INCLUDED
#define ALGEBRA_H_INCLUDED

#include <GL/gl.h>

// Calculates result of adding vectors
GLfloat *Add3(GLfloat result[3], const GLfloat v1[3], const GLfloat v2[3]);

// Calculates result of multiplying vector by a scalar
GLfloat *Scalar3(GLfloat result[3], GLfloat s, GLfloat v[3]);

// Performs: result = v1 + s*v2
GLfloat *Saxpy3(
	GLfloat       result[3],
	const GLfloat v1[3],
	GLfloat       s,
	const GLfloat v2[3]
);

// Calculate cross product of given vectors in parameter order
GLfloat *Cross3(
	GLfloat       result[restrict 3],
	const GLfloat v1[3],
	const GLfloat v2[3]
);

// Calculate dot product of given vectors
GLfloat Dot3(const GLfloat v1[3], const GLfloat v2[3]);

// Calculate length of vector
GLfloat Length3(const GLfloat v[3]);

// Calculate unit vector with same direction
GLfloat *Normalize3(GLfloat result[3], const GLfloat v[3]);

// Calculate angle in arc from v1 to v2
GLfloat Angle3(const GLfloat v1[3], const GLfloat v2[3]);

#endif // ALGEBRA_H_INCLUDED
