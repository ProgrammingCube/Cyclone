#ifndef PLATFORMS_H
#define PLATFORMS_H

#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "utils.h"
#include "gamestate.h"

#define NUM_PLATFORMS   3

int initPlatforms( GameState* gameState );

#endif