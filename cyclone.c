#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "utils.h"
//#include "datatypes.h"
#include "gameInit.h"
//#include "player.h"
#include "gamestate.h"
#include  "renderer.h"

int main( int argc, char** argv )
{
    

    GameState gameState;
    openGLInit( &argc, argv );
    GameState_Init( &gameState );

    gameState_Subscribe( &gameState );
    renderer_Subscribe( &gameState );
    gameInit();

    glutDisplayFunc( game_render );
    glutIdleFunc( game_update );
    glutReshapeFunc( game_reshape );
    //glutKeyboardFunc(handle_keyboard_input);
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

    glutMainLoop();

    gameState_Unsubscribe();
    renderer_Unsubscribe();

    return 0;
}