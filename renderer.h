#ifndef RENDERER_H
#define RENDERER_H

#include "gamestate.h"
#include <GL/glew.h>

void init_renderer(int w, int h);
// Pass the whole GameState to give the renderer access to the level data
void draw_scene(GameState* state);
void free_renderer_resources();
void reshape_renderer(int w, int h);
GLuint create_shader_program(const char* vert_shader_src, const char* frag_shader_src);

#endif // RENDERER_H