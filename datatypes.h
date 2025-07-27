#ifndef DATATYPES_H
#define DATATYPES_H

#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdbool.h>
#include "cglm/cglm.h"

typedef GLuint ShaderProgram;
typedef GLuint TextureID;

extern mat4 view_matrix;
extern mat4 projection_matrix;

#endif