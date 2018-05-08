#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "DrawUtil.h"
#include "Algebra.h"

#include "Track.h"

#define PI 3.14159265358979323846264338327950288

enum {
	Type_straight,
	Type_curved
};

static const GLfloat iUnit[3] = {1, 0, 0};

StraightTrack g_initialTrackPiece = {
	.shared = {.type = Type_straight},
	.start  = {-3, 0},
	.end    = {3, 0},
	.next   = NULL,
	.prev   = NULL
};

NetworkPos *NetworkPos_Move(NetworkPos *np, GLfloat vector)
{
	vector += np->pos;
	if (vector >= 0) {
		GLfloat length;
		while ((length = Track_GetLength(np->track)) < vector) {
			np->track = Track_GetNext(np->track);
			vector -= length;
		}
		np->pos = vector;
	} else {
		np->track = Track_GetPrevious(np->track);
		vector *= -1;
		GLfloat length;
		while ((length = Track_GetLength(np->track)) < vector) {
			np->track = Track_GetPrevious(np->track);
			vector -= length;
		}
		np->pos = length - vector;
	}
	return np;
}

static void CalcStraightDims(StraightTrack *track, StraightDims *dims)
{
	// Calculate position, length and orientation
	GLfloat start[3] = {track->start[0], 0, track->start[1]};
	GLfloat end[3] = {track->end[0], 0, track->end[1]};
	GLfloat sectionVector[3];
	Saxpy3(sectionVector, end, -1, start);
	dims->length = Length3(sectionVector);
	dims->orientation = Angle3(iUnit, sectionVector);
	if (Cross3((GLfloat [3]){0}, iUnit, sectionVector)[1] < 0) {
		dims->orientation *= -1;
	}
	Saxpy3(dims->position, start, 0.5, sectionVector);
}

StraightTrack *AllocStraightTrack(
	const GLfloat start[3],
	const GLfloat end[3],
	TrackShared   *next,
	TrackShared   *prev)
{
	StraightTrack *result = malloc(sizeof *result);
	assert(result);
	*result = (StraightTrack){
		.shared = {.type = Type_straight},
		.next   = next,
		.prev   = prev
	};
	memcpy(result->start, start, sizeof result->start);
	memcpy(result->end, end, sizeof result->end);
	CalcStraightDims(result, &result->dims);
	return result;
}

static GLfloat *Project3(GLfloat result[3], const GLfloat vec[2])
{
	result[0] = vec[0];
	result[1] = 0;
	result[2] = vec[1];
	return result;
}

// Calculate intercept of two lines: v1,v2 are on one line, v3,v4 on other.
static GLfloat *Intercept2(
	GLfloat       result[2],
	const GLfloat v1[2],
	const GLfloat v2[2],
	const GLfloat v3[2],
	const GLfloat v4[2])
{
	GLfloat d12[2]  = {v1[0] - v2[0], v1[1] - v2[1]},
	        d34[2]  = {v3[0] - v4[0], v3[1] - v4[1]},
	        divisor = d12[0]*d34[1] - d12[1]*d34[0];
	if (divisor == 0) {
		return NULL;
	}
	GLfloat e1 = v1[0]*v2[1] - v1[1]*v2[0],
	        e2 = v3[0]*v4[1] - v3[1]*v4[0];
	divisor = 1/divisor;
	result[0] = (e1*d34[0] - e2*d12[0]) * divisor;
	result[1] = (e1*d34[1] - e2*d12[1]) * divisor;
	return result;
}

static GLfloat *Project2(GLfloat result[2], const GLfloat v[3])
{
	result[0] = v[0];
	result[1] = v[2];
	return result;
}

static void CalcCurvedDims(
	CurvedTrack *track,
	CurvedDims  *dims)
{
	// Default values
	*dims = (CurvedDims){0};

	// Calculate intercept of paths
	GLfloat v1[2] = {
		track->start[0] + track->startDir[0],
		track->start[1] + track->startDir[1]
	};
	GLfloat *v2 = track->start;
	GLfloat v3[2] = {
		track->end[0] + track->endDir[0],
		track->end[1] + track->endDir[1]
	};
	GLfloat *v4 = track->end;
	GLfloat inter[3];
	Project3(inter, Intercept2((GLfloat [2]){0}, v1, v2, v3, v4));

	// Calculate intercept of normals to path lines at one distance to intercept
	GLfloat start[3], startDir[3], end[3], endDir[3];
	Project3(start, track->start);
	Normalize3(startDir, Project3((GLfloat [3]){0}, track->startDir));
	Project3(end, track->end);
	Normalize3(endDir, Project3((GLfloat [3]){0}, track->endDir));

	// Use the smaller distance to the intercept
	GLfloat startToInter[3], endToInter[3];
	Saxpy3(startToInter, inter, -1, start);
	Saxpy3(endToInter, inter, -1, end);
	GLfloat startToInterL = Length3(startToInter),
	        endToInterL   = Length3(endToInter);
	GLfloat w1[2], w2[2], w3[2], w4[2];
	if (startToInterL < endToInterL) {
		// Start is closer to intercept
		Project2(w1, start);
		w2[0] = w1[0] - startToInter[2];
		w2[1] = w1[1] + startToInter[0];
		Project2(
			w3,
			Saxpy3(
				(GLfloat [3]){0},
				end,
				(endToInterL-startToInterL)/endToInterL,
				endToInter
			)
		);
		w4[0] = w3[0] - endToInter[2];
		w4[1] = w3[1] + endToInter[0];
	} else {
		// End is closer to intercept
		Project2(
			w1,
			Saxpy3(
				(GLfloat [3]){0},
				start,
				(startToInterL-endToInterL)/startToInterL,
				startToInter
			)
		);
		w2[0] = w1[0] - startToInter[2];
		w2[1] = w1[1] + startToInter[0];
		Project2(w3, end);
		w4[0] = w3[0] - endToInter[2];
		w4[1] = w3[1] + endToInter[0];
	}
	Project3(dims->arcOrigin, Intercept2((GLfloat [2]){0}, w1, w2, w3, w4));

	// Translate and rotate end vector to get start position as origin, and
	// startDir as the i unit vector
	GLfloat endT[2] = {end[0] - start[0], end[2] - start[2]};
	GLfloat rotMat[4] = {
		startDir[0], -startDir[2],
		startDir[2], startDir[0]
	};
	GLfloat endT2[2] = {
		rotMat[0]*endT[0] + rotMat[2]*endT[1],
		rotMat[1]*endT[0] + rotMat[3]*endT[1]
	};
	// Determine if clockwise drawing
	if (endT2[1] > 0) {
		dims->clockwiseArc = true;
	}

	// Calculate arc angles
	GLfloat w1_[3], w3_[3];
	Project3(w1_, w1);
	Project3(w3_, w3);
	GLfloat arcVecStart[3];
	Saxpy3(arcVecStart, w1_, -1, dims->arcOrigin);
	dims->startAngle = Angle3(iUnit, arcVecStart);
	if (Cross3((GLfloat [3]){0}, iUnit, arcVecStart)[1] < 0) {
		dims->startAngle *= -1;
	}
	GLfloat arcVecEnd[3];
	Saxpy3(arcVecEnd, w3_, -1, dims->arcOrigin);
	GLfloat endAngle = Angle3(iUnit, arcVecEnd);
	if (Cross3((GLfloat [3]){0}, iUnit, arcVecEnd)[1] < 0) {
		endAngle *= -1;
	}
	dims->arcAngle = fmodf(360 + endAngle - dims->startAngle, 360);
	if (dims->clockwiseArc) {
		dims->arcAngle = 360 - dims->arcAngle;
	}
	// Calculate arc radius
	dims->arcRadius = Length3(arcVecStart);

	// Set up straight section
	if (startToInterL < endToInterL) {
		GLfloat arcToEnd[3];
		Saxpy3(arcToEnd, end, -1, w3_);
		Saxpy3(dims->straightSection.position, w3_, 0.5, arcToEnd);
		dims->straightSection.length = Length3(arcToEnd);
		dims->straightSection.orientation = Angle3(iUnit, arcToEnd);
		if (Cross3((GLfloat [3]){0}, iUnit, arcToEnd)[1] < 0) {
			dims->straightSection.orientation *= -1;
		}
	} else {
		GLfloat startToArc[3];
		Saxpy3(startToArc, w1_, -1, start);
		Saxpy3(dims->straightSection.position, start, 0.5, startToArc);
		dims->straightSection.length = Length3(startToArc);
		dims->straightSection.orientation = Angle3(iUnit, startToArc);
		if (Cross3((GLfloat [3]){0}, iUnit, startToArc)[1] < 0) {
			dims->straightSection.orientation *= -1;
		}
		dims->straightFirst = true;
	}

	// Calculate number of desired segments
	if (dims->arcRadius == 0) {
		dims->segments = 0;
	} else {
		static const GLfloat targetArcAngleDiff = 3,
		                     minSegmentLength   = 0.2;
		dims->arcLength = 2*PI/360 * dims->arcAngle * dims->arcRadius;
		dims->segments = dims->arcAngle / targetArcAngleDiff;
		if (dims->arcLength / dims->segments < minSegmentLength) {
			dims->segments = dims->arcLength / minSegmentLength;
			if (dims->segments == 0) {
				dims->segments = 1;
			}
		}
	}
}

CurvedTrack *AllocCurvedTrack(
	const GLfloat start[2],
	const GLfloat startDir[2],
	const GLfloat end[2],
	const GLfloat endDir[2],
	TrackShared   *next,
	TrackShared   *prev)
{
	CurvedTrack *result = malloc(sizeof *result);
	assert(result);
	*result = (CurvedTrack){
		.shared = {.type = Type_curved},
		.next   = next,
		.prev   = prev
	};
	memcpy(result->start, start, sizeof result->start);
	memcpy(result->startDir, startDir, sizeof result->startDir);
	memcpy(result->end, end, sizeof result->end);
	memcpy(result->endDir, endDir, sizeof result->endDir);
	CalcCurvedDims(result, &result->dims);
	return result;
}

static GLuint straightRailsDl = 0;
static const GLfloat whiteColor[4] = {1, 1, 1, 1},
                     blackColor[4] = {0, 0, 0, 1},
                     woodColor[4] = {0.3, 0.2, 0.15, 1},
                     metalColor[4] = {0.40, 0.35, 0.37, 1};

// Draws 3 faces (vertical sides + top) of box for train rails
static void DrawRailBox(void)
{
	glPushMatrix();
		glScalef(1, 0.05, 0.04);
		glPushMatrix();
			glRotatef(-90, 1, 0, 0);
			glTranslatef(0, 0.5, 0);
			DrawSquare();
		glPopMatrix();
		glPushMatrix();
			glTranslatef(0, 0.5, 0);
			DrawSquare();
		glPopMatrix();
		glPushMatrix();
			glRotatef(90, 1, 0, 0);
			glTranslatef(0, 0.5, 0);
			DrawSquare();
		glPopMatrix();
	glPopMatrix();
}

void InitTrack(void)
{
	CalcStraightDims(&g_initialTrackPiece, &g_initialTrackPiece.dims);

	straightRailsDl = glGenLists(1);
	assert(straightRailsDl);
	glNewList(straightRailsDl, GL_COMPILE);
		glPushMatrix();
			glTranslatef(0, 0.075, 0);
			glPushMatrix();
				glRotatef(180, 0, 1, 0);
				glTranslatef(0, 0, 0.5);
				DrawRailBox();
			glPopMatrix();
			glPushMatrix();
				glTranslatef(0, 0, 0.5);
				DrawRailBox();
			glPopMatrix();
		glPopMatrix();
	glEndList();
}

// Draw straight rails
static void DrawStraightTrackSection(
	const GLfloat position[3],
	GLfloat       orientation,
	GLfloat       length)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, metalColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, metalColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 50);
	glPushMatrix();
		glTranslatef(position[0], position[1], position[2]);
		glRotatef(orientation, 0, 1, 0);
		glScalef(length, 1, 1);
		glCallList(straightRailsDl);
	glPopMatrix();
}

static void DrawStraightTrack(StraightTrack *track)
{
	StraightDims *dims = &track->dims;
	DrawStraightTrackSection(
		dims->position,
		dims->orientation,
		dims->length
	);
}

static void DrawCurvedTrackArc(
	GLfloat  radius,
	GLfloat  startAngle,
	GLfloat  arcAngle,
	unsigned segments)
{
	glMaterialfv(GL_FRONT, GL_AMBIENT, metalColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, metalColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, whiteColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 50);

	// Draw inner-arc track face
	glBegin(GL_TRIANGLE_STRIP);
	for (unsigned i = 0; i <= segments; ++i) {
		GLfloat angle =
			startAngle + i*arcAngle/segments;
		GLfloat cosine = cosf(2*PI/360*angle), sine = sinf(2*PI/360*angle);
		glNormal3f(-cosine, 0, sine);
		glVertex3f(radius*cosine, -0.5, -radius*sine);
		glNormal3f(-cosine, 0, sine);
		glVertex3f(radius*cosine, 0.5, -radius*sine);
	}
	glEnd();
	// Draw top track face
	glBegin(GL_TRIANGLE_STRIP);
	for (unsigned i = 0; i <= segments; ++i) {
		GLfloat angle =
			startAngle + i*arcAngle/segments;
		GLfloat cosine = cosf(2*PI/360*angle), sine = sinf(2*PI/360*angle);
		glNormal3f(0, 1, 0);
		glVertex3f(radius*cosine, 0.5, -radius*sine);
		glNormal3f(0, 1, 0);
		glVertex3f((radius+0.04)*cosine, 0.5, -(radius+0.04)*sine);
	}
	glEnd();
	radius += 0.04;
	// Draw outer-arc track face
	glBegin(GL_TRIANGLE_STRIP);
	for (unsigned i = 0; i <= segments; ++i) {
		GLfloat angle =
			startAngle + i*arcAngle/segments;
		GLfloat cosine = cosf(2*PI/360*angle), sine = sinf(2*PI/360*angle);
		glNormal3f(cosine, 0, -sine);
		glVertex3f(radius*cosine, 0.5, -radius*sine);
		glNormal3f(cosine, 0, -sine);
		glVertex3f(radius*cosine, -0.5, -radius*sine);
	}
	glEnd();
}

static void DrawCurvedTrack(CurvedTrack *track)
{
	glPushMatrix();
	if (!track->renderDl) {
		track->renderDl = glGenLists(1);
		assert(track->renderDl);
		glNewList(track->renderDl, GL_COMPILE_AND_EXECUTE);
			CurvedDims *dims = &track->dims;
			StraightDims *line = &dims->straightSection;
			if (line->length != 0) {
				DrawStraightTrackSection(
					line->position,
					line->orientation,
					line->length
				);
			}
			glTranslatef(
				dims->arcOrigin[0],
				dims->arcOrigin[1] + 0.075,
				dims->arcOrigin[2]
			);
			glScalef(1, 0.05, 1);
			GLfloat startAngle = dims->startAngle;
			GLfloat radius = dims->arcRadius - 0.52;
			if (dims->clockwiseArc) {
				startAngle = dims->startAngle - dims->arcAngle;
			}
			DrawCurvedTrackArc(
				radius,
				startAngle,
				dims->arcAngle,
				dims->segments
			);
			radius += 1;
			DrawCurvedTrackArc(
				radius,
				startAngle,
				dims->arcAngle,
				dims->segments
			);
		glEndList();
	} else {
		glCallList(track->renderDl);
	}
	glPopMatrix();
}

void Track_Draw(TrackShared *track)
{
	switch (track->type) {
	case Type_straight:
		DrawStraightTrack((StraightTrack *)track);
		break;
	case Type_curved:
		DrawCurvedTrack((CurvedTrack *)track);
		break;
	default:
		abort();
	}
}

static GLfloat StraightTrack_GetLength(StraightTrack *track)
{
	return track->dims.length;
}

static GLfloat CurvedTrack_GetLength(CurvedTrack *track)
{
	return   track->dims.arcLength
	       + track->dims.straightSection.length;
}

GLfloat Track_GetLength(TrackShared *track)
{
	switch (track->type) {
	case Type_straight:
		return StraightTrack_GetLength((StraightTrack *)track);
	case Type_curved:
		return CurvedTrack_GetLength((CurvedTrack *)track);
	default:
		abort();
	}
}

static TrackShared *StraightTrack_GetNext(StraightTrack *track)
{
	return track->next;
}

static TrackShared *CurvedTrack_GetNext(CurvedTrack *track)
{
	return track->next;
}

TrackShared *Track_GetNext(TrackShared *track)
{
	switch (track->type) {
	case Type_straight:
		return StraightTrack_GetNext((StraightTrack *)track);
	case Type_curved:
		return CurvedTrack_GetNext((CurvedTrack *)track);
	default:
		abort();
	}
}

static TrackShared *StraightTrack_GetPrevious(StraightTrack *track)
{
	return track->prev;
}

static TrackShared *CurvedTrack_GetPrevious(CurvedTrack *track)
{
	return track->prev;
}

TrackShared *Track_GetPrevious(TrackShared *track)
{
	switch (track->type) {
	case Type_straight:
		return StraightTrack_GetPrevious((StraightTrack *)track);
	case Type_curved:
		return CurvedTrack_GetPrevious((CurvedTrack *)track);
	default:
		abort();
	}
}

void StraightTrack_GetCoords(
	StraightTrack *track,
	GLfloat       coords[3],
	GLfloat       pos)
{
	GLfloat start[3], end[3], forwards[3];
	Project3(start, track->start);
	Project3(end, track->end);
	Normalize3(forwards, Saxpy3((GLfloat [3]){0}, end, -1, start));
	Saxpy3(coords, start, pos, forwards);
}

void CurvedTrack_GetCoords(CurvedTrack *track, GLfloat coords[3], GLfloat pos)
{
	GLfloat forwards[3];
	CurvedDims *dims = &track->dims;
	if (   (dims->straightFirst && pos <= dims->straightSection.length)
	    || (!dims->straightFirst && pos > dims->arcLength))
	{
		if (!dims->straightFirst) {
			pos -= dims->arcLength;
		}
		forwards[0] =
			cosf(2*PI/360 * dims->straightSection.orientation);
		forwards[1] = 0;
		forwards[2] =
			-sinf(2*PI/360 * dims->straightSection.orientation);
		GLfloat start[3];
		Saxpy3(
			start,
			dims->straightSection.position,
			-0.5*dims->straightSection.length,
			forwards
		);
		Saxpy3(coords, start, pos, forwards);
	} else {
		if (dims->straightFirst) {
			pos -= dims->straightSection.length;
		}
		GLfloat startAngle = dims->startAngle;
		GLfloat angle;
		if (!dims->clockwiseArc) {
			angle =   startAngle
			        + dims->arcAngle * pos / dims->arcLength;
		} else {
			angle =   startAngle
			        - dims->arcAngle * pos / dims->arcLength;
		}
		coords[0] =   dims->arcOrigin[0]
		            + dims->arcRadius*cosf(2*PI/360 * angle);
		coords[1] =   dims->arcOrigin[1];
		coords[2] =   dims->arcOrigin[2]
		            - dims->arcRadius*sinf(2*PI/360 * angle);
	}
}

void Track_GetCoords(TrackShared *track, GLfloat coords[3], GLfloat pos)
{
	switch (track->type) {
	case Type_straight:
		StraightTrack_GetCoords((StraightTrack *)track, coords, pos);
		break;
	case Type_curved:
		CurvedTrack_GetCoords((CurvedTrack *)track, coords, pos);
		break;
	default:
		abort();
	}
}

static void DrawSlatsStep(NetworkPos *pos)
{
	GLfloat start[3], end[3], diff[3];
	NetworkPos_Move(pos, -0.001);
	Track_GetCoords(pos->track, start, pos->pos);
	NetworkPos_Move(pos, 0.001);
	Track_GetCoords(pos->track, end, pos->pos);
	Saxpy3(diff, end, -1, start);
	glPushMatrix();
		glTranslatef(end[0], end[1], end[2]);
		GLfloat angle = Angle3(iUnit, diff);
		if (Cross3((GLfloat [3]){0}, iUnit, diff)[1] < 0) {
			angle *= -1;
		}
		glRotatef(angle, 0, 1, 0);
		glScalef(0.2, 0.0375, 1.2);
		glBegin(GL_QUADS);
			glNormal3f(0, 0, 1);
			glVertex3f(0.5, 0, 0.5);
			glVertex3f(0.5, 1, 0.5);
			glVertex3f(-0.5, 1, 0.5);
			glVertex3f(-0.5, 0, 0.5);

			glNormal3f(-1, 0, 0);
			glVertex3f(-0.5, 0, 0.5);
			glVertex3f(-0.5, 1, 0.5);
			glVertex3f(-0.5, 1, -0.5);
			glVertex3f(-0.5, 0, -0.5);

			glNormal3f(0, 0, -1);
			glVertex3f(-0.5, 0, -0.5);
			glVertex3f(-0.5, 1, -0.5);
			glVertex3f(0.5, 1, -0.5);
			glVertex3f(0.5, 0, -0.5);

			glNormal3f(1, 0, 0);
			glVertex3f(0.5, 0, -0.5);
			glVertex3f(0.5, 1, -0.5);
			glVertex3f(0.5, 1, 0.5);
			glVertex3f(0.5, 0, 0.5);

			glNormal3f(0, 1, 0);
			glVertex3f(0.5, 1, 0.5);
			glVertex3f(0.5, 1, -0.5);
			glVertex3f(-0.5, 1, -0.5);
			glVertex3f(-0.5, 1, 0.5);
		glEnd();
	glPopMatrix();
}

// Draws wooden slats in track network
void DrawSlats(GLfloat minDistance)
{
	// Compile slats on first run
	static GLuint slatsDl = 0;
	if (slatsDl) {
		glPushMatrix();
			glCallList(slatsDl);
		glPopMatrix();
		return;
	}
	slatsDl = glGenLists(1);
	assert(slatsDl);
	glPushMatrix();
	glNewList(slatsDl, GL_COMPILE_AND_EXECUTE);

	// Set to slat material
	glMaterialfv(GL_FRONT, GL_AMBIENT, woodColor);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, woodColor);
	glMaterialfv(GL_FRONT, GL_SPECULAR, blackColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 0);

	// Calculate total track length
	TrackShared *track = (TrackShared *)&g_initialTrackPiece;
	GLfloat length = Track_GetLength(track);
	track = Track_GetNext(track);
	while (track != (TrackShared *)&g_initialTrackPiece) {
		length += Track_GetLength(track);
		track = Track_GetNext(track);
	}
	// Find actual target distance
	minDistance = length / floorf(length / minDistance);

	// Draw slats for whole track
	NetworkPos pos = {(TrackShared *)&g_initialTrackPiece, 0};
	while (pos.track == (TrackShared *)&g_initialTrackPiece) {
		DrawSlatsStep(&pos);
		NetworkPos_Move(&pos, minDistance);
	}
	while (pos.track != (TrackShared *)&g_initialTrackPiece) {
		DrawSlatsStep(&pos);
		NetworkPos_Move(&pos, minDistance);
	}

	glEndList();
	glPopMatrix();
}
