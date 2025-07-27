#include "renderer.h"

static GameState* s_gameState;
static Object* s_player;

void renderer_Subscribe( GameState* gameState )
{
    s_gameState = gameState;
    s_player = &gameState->player;
}

void renderer_Unsubscribe()
{
    s_gameState = NULL;
}

void game_render()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glClearColor( 0.1f, 0.1f, 0.15f, 1.0f );

    // view matrix ( camera's position and orientation )
    glm_lookat(
        (vec3){0.0f, 0.0f, 5.0f}, // Camera is 5 units back
        (vec3){0.0f, 0.0f, 0.0f}, // Looking at the origin
        (vec3){0.0f, 1.0f, 0.0f}, // "Up" is the Y-axis
        view_matrix
    );

    // projection matrix ( lens )
    glm_perspective(
        glm_rad( 45.0f ),
        ( float )GLUT_WINDOW_WIDTH / ( float )GLUT_WINDOW_HEIGHT,
        0.1f,
        100.0f,
        projection_matrix
    );

    drawPlayer( s_player );

    glBindVertexArray( 0 );
    glUseProgram( 0 );
    glutSwapBuffers();
}

void game_update()
{
    //
    glutPostRedisplay();
}

void game_reshape( int w, int h )
{
    //
    glutPostRedisplay();
}
