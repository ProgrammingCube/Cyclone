#ifndef OBJECT_H
#define OBJECT_H

#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "datatypes.h"

#define NUM_SQUARE_VERTS    12
#define NUM_SQUARE_INDCS    6

static GLfloat square_vertices[ NUM_SQUARE_VERTS ];
static GLuint square_indices[ NUM_SQUARE_INDCS ];

typedef vec3 Position;

typedef struct sMesh
{
    GLuint vbo;     // vertex buffer object
    GLuint ibo;     // index buffer object
    GLuint ebo;     // element buffer object
    GLuint vao;     // vertex array object
    GLint posAttrib;
    GLsizeiptr vbSize;
    GLsizeiptr ebSize;

} Mesh;

typedef enum eObjectType
{
    TYPE_PLAYER,
    TYPE_PLATFORM,
    TYPE_JUMPPAD,
    TYPE_SQUARE
} ObjectType;

typedef struct sObject
{
    ObjectType type;
    Mesh mesh;
    ShaderProgram shader;
    Position pos;
    GLfloat width, height, length;
} Object;

Object createObject( ObjectType type );
void initObject( Object* obj, ObjectType type );
int setupGeometry( Object* obj );

#endif