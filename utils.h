#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "cglm/cglm.h"

#define GL_LOG_LENGTH   512

char* readFile( char* file_name );
char* loadShaderSource( char* file_name );
GLuint compileShader( GLenum type, char* source_file );
GLuint createShaderProgram( char* vertexShader_source, char* fragmentShader_source );

#endif