#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "gameInit.h"
#include "utils.h"
#include "gamestate.h"
//#include "datatypes.h"
#include "player.h"
#include "platforms.h"

int GAME_WINDOW_WIDTH  = 720;
int GAME_WINDOW_HEIGHT = 1280;

#define ESC_KEY 27

//static GLuint player_shader_program;

static GameState* s_gameState = NULL;
static Object* s_player;

int openGLInit( int* argc, char** argv )
{
    glutInit( argc, argv );
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    glutInitWindowSize( GAME_WINDOW_HEIGHT, GAME_WINDOW_WIDTH );
    glutInitWindowPosition( 1000, 1200 );
    glutInitContextVersion( 3, 3 );
    glutCreateWindow( "Cyclone3D" );

    if ( glewInit() != GLEW_OK )
    {
        fprintf( stderr, "Failed to initialize GLEW\n" );
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    fprintf( stdout, "Using GLEW %s\n", glewGetString( GLEW_VERSION ) );
    fprintf( stdout, "Using OpenGL %s\n", glGetString( GL_VERSION ) );

    glutDisplayFunc( render );
    glutReshapeFunc( reshape );
    glutTimerFunc( 16, update, 0 );
    glutKeyboardFunc( keyboardDown );
    glutKeyboardUpFunc( keyboardUp );
    glutCloseFunc( onClose );
    glutIgnoreKeyRepeat( 1 );
    return 1;
}

void gameState_Subscribe( GameState* gameState )
{
    s_gameState = gameState;
    s_player = &s_gameState->player;
}

void gameState_Unsubscribe()
{
    s_gameState = NULL;
}

int gameInit()
{
    if ( s_gameState == NULL )
    {
        fprintf( stderr, "Error: gameInit context not set!\n" );
        return -1;
    }
    

    initObject( s_player, TYPE_PLAYER );

    Vector_Init( &s_gameState->platforms, NUM_PLATFORMS );

    // read level file

    // set up platforms, spikes, jumppads
    initPlatforms( s_gameState );

    return 1;
}

void render()
{
    

}
void update( int value )
{
    //
    glutTimerFunc( 16, update, 0 );
}
void reshape( int w, int h )
{
    //
}
void keyboardDown( unsigned char key, int x, int y )
{
    switch ( key )
    {
        case ESC_KEY:
            glutLeaveMainLoop();
            break;
        default:
            break;
    }
}
void keyboardUp( unsigned char key, int x, int y )
{
    //
}
void onClose( void )
{
    /*
    if ( !s_player->shader )
        glDeleteProgram( s_player->shader );
        */
}