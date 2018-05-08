#include <math.h>
#include <assert.h>

#include "Algebra.h"

#define PI 3.14159265358979323846264338327950288

GLfloat *Add3(GLfloat result[3], const GLfloat v1[3], const GLfloat v2[3])
{
	for (unsigned i = 0; i < 3; ++i) {
		result[i] = v1[i] + v2[i];
	}
	return result;
}

GLfloat *Scalar3(GLfloat result[3], GLfloat s, GLfloat v[3])
{
	for (unsigned i = 0; i < 3; ++i) {
		result[i] = s*v[i];
	}
	return result;
}

GLfloat *Saxpy3(
	GLfloat       result[3],
	const GLfloat v1[3],
	GLfloat       s,
	const GLfloat v2[3]
)
{
	for (unsigned i = 0; i < 3; ++i) {
		result[i] = v1[i] + s*v2[i];
	}
	return result;
}

GLfloat *Cross3(
	GLfloat       result[restrict 3],
	const GLfloat v1[3],
	const GLfloat v2[3])
{
	result[0] = v1[1]*v2[2] - v1[2]*v2[1];
	result[1] = v1[2]*v2[0] - v1[0]*v2[2];
	result[2] = v1[0]*v2[1] - v1[1]*v2[0];
	return result;
}

GLfloat Dot3(const GLfloat v1[3], const GLfloat v2[3])
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

GLfloat Length3(const GLfloat v[3])
{
	return sqrtf(Dot3(v, v));
}

GLfloat *Normalize3(GLfloat result[3], const GLfloat v[3])
{
	GLfloat length = Length3(v);
	for (unsigned i = 0; i < 3; ++i) {
		result[i] = v[i]/length;
	}
	return result;
}

GLfloat Angle3(const GLfloat v1[3], const GLfloat v2[3])
{
	return 180*acosf(Dot3(v1, v2) / (Length3(v1) * Length3(v2))) / PI;
}
