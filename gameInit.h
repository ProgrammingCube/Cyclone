#ifndef GAME_INIT_H
#define GAME_INIT_H

#include "gamestate.h"

extern int GAME_WINDOW_WIDTH;
extern int GAME_WINDOW_HEIGHT;

int openGLInit( int* argc, char** argv );

void gameState_Subscribe( GameState* gameState );
void gameState_Unsubscribe( void );

int gameInit( void );

void render( void );
void update( int value );
void reshape( int w, int h );
void keyboardDown( unsigned char key, int x, int y );
void keyboardUp( unsigned char key, int x, int y );
void onClose( void );

#endif