#include "player.h"

mat4 view_matrix;
mat4 projection_matrix;

int initPlayer( Object* player )
{
    setupGeometry( player );

    //player->pos = ( vec3 ){ 0.0f, 5.0f, -1.0f };

    player->width = 0.25f;
    player->length = 0.25f;
    player->height = 0.25f;

    char* player_vertexSource_file = "playerVertex.glsl";
    char* player_fragmentSource_file = "playerFragment.glsl";

    player->shader =  createShaderProgram( player_vertexSource_file,
                                            player_fragmentSource_file
                                            );

    return 0;
}

int drawPlayer( Object* player )
{
    // assume player is always alive, add that later
    glUseProgram( player->shader );

    glUniformMatrix4fv( glGetUniformLocation( player->shader, "view_matrix" ), 1, GL_FALSE, ( float* )view_matrix );
    glUniformMatrix4fv( glGetUniformLocation( player->shader, "projection_matrix" ), 1, GL_FALSE, ( float* )projection_matrix );
    
    mat4 model_matrix;
    glm_mat4_identity( model_matrix );
    float time = glutGet( GLUT_ELAPSED_TIME ) / 1000.0f;
    glm_rotate( model_matrix, time, ( vec3 ){ 0.0f, 0.0f, 1.0f } );

    glUniformMatrix4fv( glGetUniformLocation( player->shader, "model_matrix"), 1, GL_FALSE, ( float* )model_matrix );

    glBindVertexArray( player->mesh.vao );
    glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

    glUseProgram( 0 );
    return 0;
}