#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "datatypes.h"
#include "object.h"
#include "player.h"
#include "platforms.h"

void renderer_Subscribe( GameState* gameState );
void renderer_Unsubscribe( void );

void game_render( void );

void game_update( void );

void game_reshape( int w, int h );

#endif