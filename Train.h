#ifndef DRAW_TRAIN_H_INCLUDED
#define DRAW_TRAIN_H_INCLUDED

#include <GL/gl.h>
#include "Track.h"

extern unsigned g_nCarriages;
extern GLfloat g_trainSpeed;
extern NetworkPos g_trainPos;

void InitTrain(void);

void DrawTrain(void);

#endif // DRAW_TRAIN_H_INCLUDED
