#ifndef TRACK_H_INCLUDED
#define TRACK_H_INCLUDED

#include <stdbool.h>
#include <GL/gl.h>

// Must be called before using other track functions
void InitTrack(void);

// 'Parent' type for track pieces
typedef struct {
	unsigned type;
} TrackShared;

// Straight track pre-calculated dimensions
typedef struct {
	GLfloat position[3],
	        length,
	        orientation;
} StraightDims;

// Specialized type for straight track pieces
typedef struct {
	TrackShared  shared;
	GLfloat      start[2],
	             end[2];
	TrackShared  *next,
	             *prev;
	StraightDims dims;
} StraightTrack;

// Curved track pre-calculated dimensions
typedef struct {
	StraightDims straightSection;
	GLfloat      arcOrigin[3],
	             startAngle,
	             arcAngle,
	             arcLength,
	             arcRadius;
	unsigned     segments;
	bool         straightFirst,
	             clockwiseArc;
} CurvedDims;

// Specialized type for curved track pieces
typedef struct {
	TrackShared shared;
	GLfloat     start[2],
	            startDir[2],
	            end[2],
	            endDir[2];
	TrackShared *next,
	            *prev;
	GLuint      renderDl;
	CurvedDims  dims;
} CurvedTrack;

// Represents a position on the track network, occupied by a train or carriage
typedef struct {
	TrackShared *track; // Current track piece
	GLfloat     pos;    // Current length through the current track piece
} NetworkPos;

// Move position along track by given vector (i.e. negative to go backwards)
NetworkPos *NetworkPos_Move(NetworkPos *np, GLfloat vector);

extern StraightTrack g_initialTrackPiece;

// Allocates new straight section of track that runs from start to end, with
// next and previous track sections (or null pointer).
// Can free with free() or realloc().
StraightTrack *AllocStraightTrack(
	const GLfloat start[2],
	const GLfloat end[2],
	TrackShared   *next,
	TrackShared   *prev
);

// Allocates new curved section of track that runs from start to end, with next
// and previous track sections (or null pointer).
// Angle between startDir and endDir must be in (0, 90], in either direction.
// Can free with free() or realloc().
CurvedTrack *AllocCurvedTrack(
	const GLfloat start[2],
	const GLfloat startDir[2],
	const GLfloat end[2],
	const GLfloat endDir[2],
	TrackShared   *next,
	TrackShared   *prev
);

// Renders a section of track
void Track_Draw(TrackShared *track);

// Gets the length of a section of track
GLfloat Track_GetLength(TrackShared *track);

// Gets the next section of track in a network
TrackShared *Track_GetNext(TrackShared *track);

// Gets the previous section of track in a network
TrackShared *Track_GetPrevious(TrackShared *track);

// Gets the coordinates of distance `pos` along this track piece
void Track_GetCoords(
	TrackShared *track,
	GLfloat     coords[3], // Stores coords of point on track
	GLfloat     pos
);

// Draws wooden slats in track network
void DrawSlats(GLfloat minDistance);

#endif // TRACK_H_INCLUDED
