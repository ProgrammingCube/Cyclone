#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "utils.h"
#include "datatypes.h"
#include "object.h"



int initPlayer( Object* player );
int drawPlayer( Object* player );

#endif