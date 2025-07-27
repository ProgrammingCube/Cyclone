#include "object.h"
#include "player.h"

static GLfloat square_vertices[ NUM_SQUARE_VERTS ] = {
    0.5f,  0.5f, 0.0f,  // Top Right
    0.5f, -0.5f, 0.0f,  // Bottom Right
    -0.5f, -0.5f, 0.0f,  // Bottom Left
    -0.5f,  0.5f, 0.0f   // Top Left
};

static GLuint square_indices[ NUM_SQUARE_INDCS ] = {
        0, 1, 3,  // First Triangle
        1, 2, 3   // Second Triangle
};

void initObject( Object* obj, ObjectType type )
{
    obj->type = type;
    switch ( type )
    {
        case TYPE_PLAYER:
            initPlayer( obj );
            break;
        case TYPE_PLATFORM:
            break;
        default:
            break;
    }
}

Object createObject( ObjectType type )
{
    Object obj;
    obj.type = type;
    switch ( type )
    {
        case TYPE_SQUARE:
        case TYPE_PLAYER:
            obj.length = 5;
            obj.height = 5;
            obj.width = 5;
            obj.pos[0] = 0.0f;
            obj.pos[1] = 2.0f;
            obj.pos[2] = -2.0f;
            break;
        case TYPE_PLATFORM:
            obj.length = 10;
            obj.height = 1;
            obj.width = 5;
            obj.pos[0] = 0.0f;
            obj.pos[1] = 0.0f;
            obj.pos[2] = 0.0f;
            break;
        case TYPE_JUMPPAD:
            break;
        default:
            break;
    }
    return obj;
}

int setupGeometry( Object* obj )
{
    GLfloat* _vertices;
    GLuint* _indices;
    //GLsizeiptr vertex_size, index_size;

    switch ( obj->type )
    {
        case TYPE_PLAYER:
        case TYPE_SQUARE:
            _vertices = square_vertices;
            _indices = square_indices;
            obj->mesh.vbSize = ( GLsizeiptr )( ( NUM_SQUARE_VERTS ) * sizeof( GLfloat ) );
            obj->mesh.ebSize = ( GLsizeiptr )( ( NUM_SQUARE_INDCS ) * sizeof( GLuint ) );
            break;
        default:
            break;
    }

    glGenVertexArrays( 1, &obj->mesh.vao );
    glBindVertexArray( obj->mesh.vao );

    glGenBuffers( 1, &obj->mesh.vbo );
    glBindBuffer( GL_ARRAY_BUFFER, obj->mesh.vbo );
    glBufferData( GL_ARRAY_BUFFER, obj->mesh.vbSize, _vertices, GL_STATIC_DRAW );

    glGenBuffers( 1, &obj->mesh.ebo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, obj->mesh.ebo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, obj->mesh.ebSize, _indices, GL_STATIC_DRAW );

    obj->mesh.posAttrib = glGetAttribLocation( obj->shader, "aPos");
    glEnableVertexAttribArray( obj->mesh.posAttrib );
    glVertexAttribPointer( obj->mesh.posAttrib, 3, GL_FLOAT, GL_FALSE, 0, ( void* )0 );

    glBindVertexArray( 0 );

    return 0;
}