#include "renderer.h"
#include "math_utils.h"
#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define PI 3.1415926535f
#define CONE_SEGMENTS 16
#define CIRCLE_SEGMENTS 24

const float SPIKE_SCALE_HEIGHT = 2.0f; // Twice player height (1.0 * 2)
const float SPIKE_SCALE_RADIUS = 0.375f; // 75% of player width (1.0 * 0.75 / 2)
const float SPIKE_OUTLINE_THICKNESS = 1.15f; // 115% scale for the outline
const float FADE_START_DISTANCE = 15.0f; // Must match fog start
const float FADE_END_DISTANCE = 80.0f;   // Must match fog end

// Shader Programs
static GLuint player_shader, wireframe_shader, emissive_shader, spike_shader, jumppad_shader, trail_shader;
// VBO/VAO handles
static GLuint cube_vao, cube_vbo;
static GLuint circle_vao, circle_vbo;
static GLuint platform_edges_vao, platform_edges_vbo;
static GLuint cone_vao, cone_vbo, cone_ebo;
static int cone_indices;



// Uniform locations (add new ones)
static GLint player_model_loc, player_view_loc, player_proj_loc;
static GLint wireframe_model_loc, wireframe_view_loc, wireframe_proj_loc, wireframe_color_loc;
static GLint emissive_model_loc, emissive_view_loc, emissive_proj_loc;
static GLint emissive_color_loc, emissive_fade_start_loc, emissive_fade_end_loc;
static GLint spike_model_loc, spike_view_loc, spike_proj_loc;
static GLint jumppad_model_loc, jumppad_view_loc, jumppad_proj_loc;
static GLint trail_model_loc, trail_view_loc, trail_proj_loc, trail_color_loc;

// Projection Matrix
static float projection_matrix[16];

// --- SHADER SOURCE CODE ---

const char* vertex_shader_src =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "varying vec3 FragPos;\n"
    "void main() {\n"
    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "}\n";

const char* player_fragment_src =
    "#version 120\n"
    "varying vec3 FragPos;\n"
    "void main() {\n"
    "    // Simple blue color with a fake specular highlight effect\n"
    "    vec3 lightPos = vec3(0.0, 5.0, 0.0);\n"
    "    vec3 norm = normalize(FragPos);\n"
    "    vec3 lightDir = normalize(lightPos - FragPos);\n"
    "    float spec = pow(max(dot(norm, lightDir), 0.0), 16);\n"
    "    vec3 base_color = vec3(0.1, 0.4, 1.0);\n"
    "    vec3 specular_color = vec3(1.0, 1.0, 1.0);\n"
    "    vec3 final_color = base_color + specular_color * spec;\n"
    "    gl_FragColor = vec4(final_color, 1.0);\n"
    "}\n";

const char* trail_fragment_src =
    "#version 120\n"
    "uniform vec4 emissionColor;\n"
    "void main() {\n"
    "    gl_FragColor = emissionColor;\n"
    "}\n";

const char* wireframe_fragment_src =
    "#version 120\n"
    "uniform vec4 emissionColor;\n"
    "void main() { gl_FragColor = emissionColor; }\n";

const char* emissive_vertex_src =
    "#version 120\n"
    "attribute vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "varying float v_camera_dist;\n" // Pass distance to fragment shader
    "void main() {\n"
    "    vec4 view_space_pos = view * model * vec4(aPos, 1.0);\n"
    "    v_camera_dist = length(view_space_pos.xyz);\n" // Calculate distance
    "    gl_Position = projection * view_space_pos;\n"
    "}\n";

// This new fragment shader uses the distance to fade the glow.
const char* emissive_fragment_src =
    "#version 120\n"
    "uniform vec4 emissionColor;\n"
    "uniform float fade_start_dist;\n"
    "uniform float fade_end_dist;\n"
    "varying float v_camera_dist;\n"
    "void main() {\n"
    "    // Smoothstep creates a nice, soft fade between the start and end distances.\n"
    "    float fade_factor = 1.0 - smoothstep(fade_start_dist, fade_end_dist, v_camera_dist);\n"
    "    gl_FragColor = vec4(emissionColor.rgb, emissionColor.a * fade_factor);\n"
    "}\n";

const char* spike_fragment_src =
    "#version 120\n"
    "varying vec3 FragPos;\n"
    "void main() {\n"
    "    vec3 lightPos = vec3(0.0, 5.0, 5.0);\n"
    "    vec3 norm = normalize(FragPos);\n"
    "    vec3 lightDir = normalize(lightPos - FragPos);\n"
    "    float spec = pow(max(dot(norm, lightDir), 0.0), 32);\n"
    "    vec3 base_color = vec3(1.0, 0.1, 0.1);\n"
    "    vec3 final_color = base_color + vec3(1.0) * spec;\n"
    "    gl_FragColor = vec4(final_color, 1.0);\n"
    "}\n";

const char* jumppad_fragment_src =
    "#version 120\n"
    "void main() { gl_FragColor = vec4(0.5, 0.8, 1.0, 1.0); }\n";

// --- MATRIX HELPER FUNCTIONS ---
static void mat4_identity(float* m) { memset(m, 0, 16 * sizeof(float)); m[0] = m[5] = m[10] = m[15] = 1.0f; }
static void mat4_translate(float* m, float x, float y, float z) { mat4_identity(m); m[12] = x; m[13] = y; m[14] = z; }
//static void mat4_scale(float* m, float x, float y, float z) { mat4_identity(m); m[0]=x; m[5]=y; m[10]=z; }
//static void mat4_multiply(float* res, float* a, float* b) { /* standard 4x4 multiply */ }
static void mat4_perspective(float* m, float fov_y, float aspect, float z_near, float z_far) {
    float const a = 1.f / tan(fov_y / 2.f);
    m[0] = a / aspect; m[1] = 0.f; m[2] = 0.f; m[3] = 0.f;
    m[4] = 0.f; m[5] = a; m[6] = 0.f; m[7] = 0.f;
    m[8] = 0.f; m[9] = 0.f; m[10] = -((z_far + z_near) / (z_far - z_near)); m[11] = -1.f;
    m[12] = 0.f; m[13] = 0.f; m[14] = -((2.f * z_far * z_near) / (z_far - z_near)); m[15] = 0.f;
}
static void mat4_lookAt(float* m, vec3 eye, vec3 center, vec3 up) {
    vec3 f = { center.x - eye.x, center.y - eye.y, center.z - eye.z };
    float f_len = sqrt(f.x*f.x + f.y*f.y + f.z*f.z); f.x/=f_len; f.y/=f_len; f.z/=f_len;
    vec3 s = { f.y * up.z - f.z * up.y, f.z * up.x - f.x * up.z, f.x * up.y - f.y * up.x };
    float s_len = sqrt(s.x*s.x + s.y*s.y + s.z*s.z); s.x/=s_len; s.y/=s_len; s.z/=s_len;
    vec3 u = { s.y * f.z - s.z * f.y, s.z * f.x - s.x * f.z, s.x * f.y - s.y * f.x };
    m[0] = s.x; m[4] = s.y; m[8] = s.z; m[12] = -(s.x * eye.x + s.y * eye.y + s.z * eye.z);
    m[1] = u.x; m[5] = u.y; m[9] = u.z; m[13] = -(u.x * eye.x + u.y * eye.y + u.z * eye.z);
    m[2] = -f.x; m[6] = -f.y; m[10] = -f.z; m[14] = f.x * eye.x + f.y * eye.y + f.z * eye.z;
    m[3] = 0.f; m[7] = 0.f; m[11] = 0.f; m[15] = 1.f;
}

// --- MATRIX HELPER FUNCTIONS ---
// ... (identity, translate, perspective, lookAt are the same) ...

static void mat4_scale(float* m, float x, float y, float z) { 
    mat4_identity(m); 
    m[0] = x; 
    m[5] = y; 
    m[10] = z; 
}

// *** NEWLY IMPLEMENTED FUNCTION ***
static void mat4_multiply(float* res, float* a, float* b) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++) {
                res[i * 4 + j] += a[j + k * 4] * b[i * 4 + k];
            }
        }
    }
}

// --- GEOMETRY SETUP ---

// Creates the 12 edges of a unit cube (size 1x1x1)
void setup_platform_edge_geometry() {
    float h = 0.5f; // half-size for a unit cube
    float vertices[] = {
        // Bottom face
        -h, -h, -h,  h, -h, -h,   h, -h, -h,  h, -h,  h,
         h, -h,  h, -h, -h,  h,  -h, -h,  h, -h, -h, -h,
        // Top face
        -h,  h, -h,  h,  h, -h,   h,  h, -h,  h,  h,  h,
         h,  h,  h, -h,  h,  h,  -h,  h,  h, -h,  h, -h,
        // Vertical edges
        -h, -h, -h, -h,  h, -h,   h, -h, -h,  h,  h, -h,
         h, -h,  h,  h,  h,  h,  -h, -h,  h, -h,  h,  h
    };

    glGenVertexArrays(1, &platform_edges_vao);
    glBindVertexArray(platform_edges_vao);
    glGenBuffers(1, &platform_edges_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, platform_edges_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind to prevent state leakage
}

// Creates the geometry for a cone
void setup_cone_geometry() {
    float vertices[(CONE_SEGMENTS + 2) * 3];
    GLuint indices_data[CONE_SEGMENTS * 2 * 3];
    cone_indices = 0;

    vertices[0] = 0.0f; vertices[1] = 1.0f; vertices[2] = 0.0f; // Apex
    vertices[3] = 0.0f; vertices[4] = 0.0f; vertices[5] = 0.0f; // Base center

    for (int i = 0; i < CONE_SEGMENTS; i++) {
        float angle = 2.0f * PI * i / CONE_SEGMENTS;
        vertices[(i + 2) * 3 + 0] = cos(angle);
        vertices[(i + 2) * 3 + 1] = 0.0f;
        vertices[(i + 2) * 3 + 2] = sin(angle);
    }

    for (int i = 0; i < CONE_SEGMENTS; i++) {
        int current = i + 2;
        int next = ((i + 1) % CONE_SEGMENTS) + 2;
        indices_data[cone_indices++] = 0; indices_data[cone_indices++] = current; indices_data[cone_indices++] = next;
        indices_data[cone_indices++] = 1; indices_data[cone_indices++] = next; indices_data[cone_indices++] = current;
    }

    glGenVertexArrays(1, &cone_vao);
    glBindVertexArray(cone_vao);
    glGenBuffers(1, &cone_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, cone_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glGenBuffers(1, &cone_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cone_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cone_indices * sizeof(GLuint), indices_data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind to prevent state leakage
}

// *** JUMP PAD FIX ***
// Correctly generates vertices for a closed circle fan
void setup_circle_geometry() {
    // We need Center + N edge points + first edge point again to close the fan
    float vertices[(CIRCLE_SEGMENTS + 2) * 3];
    vertices[0] = 0.0f; vertices[1] = 0.0f; vertices[2] = 0.0f; // Center vertex

    for (int i = CIRCLE_SEGMENTS; i >= 0; --i) {
        float angle = 2.0f * PI * i / CIRCLE_SEGMENTS;
        vertices[( (CIRCLE_SEGMENTS - i) + 1) * 3 + 0] = cos(angle);
        vertices[( (CIRCLE_SEGMENTS - i) + 1) * 3 + 1] = 0.0f;
        vertices[( (CIRCLE_SEGMENTS - i) + 1) * 3 + 2] = sin(angle);
    }

    glGenVertexArrays(1, &circle_vao);
    glBindVertexArray(circle_vao);
    glGenBuffers(1, &circle_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, circle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0); // Unbind to prevent state leakage
}

// --- RENDERER IMPLEMENTATION ---

void init_renderer(int w, int h) {
    player_shader = create_shader_program(vertex_shader_src, player_fragment_src);
    trail_shader = create_shader_program(vertex_shader_src, trail_fragment_src);

    player_shader = create_shader_program(vertex_shader_src, player_fragment_src);
    trail_shader = create_shader_program(vertex_shader_src, trail_fragment_src);

    wireframe_shader = create_shader_program(vertex_shader_src, wireframe_fragment_src);
    emissive_shader = create_shader_program(emissive_vertex_src, emissive_fragment_src);

    spike_shader = create_shader_program(vertex_shader_src, spike_fragment_src);
    jumppad_shader = create_shader_program(vertex_shader_src, jumppad_fragment_src);

    player_model_loc = glGetUniformLocation(player_shader, "model");
    player_view_loc = glGetUniformLocation(player_shader, "view");
    player_proj_loc = glGetUniformLocation(player_shader, "projection");

    trail_model_loc = glGetUniformLocation(trail_shader, "model");
    trail_view_loc = glGetUniformLocation(trail_shader, "view");
    trail_proj_loc = glGetUniformLocation(trail_shader, "projection");
    trail_color_loc = glGetUniformLocation(trail_shader, "emissionColor");

    emissive_model_loc = glGetUniformLocation(emissive_shader, "model");
    emissive_view_loc = glGetUniformLocation(emissive_shader, "view");
    emissive_proj_loc = glGetUniformLocation(emissive_shader, "projection");
    emissive_color_loc = glGetUniformLocation(emissive_shader, "emissionColor");
    emissive_fade_start_loc = glGetUniformLocation(emissive_shader, "fade_start_dist");
    emissive_fade_end_loc = glGetUniformLocation(emissive_shader, "fade_end_dist");
    
    spike_model_loc = glGetUniformLocation(spike_shader, "model");
    spike_view_loc = glGetUniformLocation(spike_shader, "view");
    spike_proj_loc = glGetUniformLocation(spike_shader, "projection");

    float vertices[] = {
        -0.5f,-0.5f,-0.5f, -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f,  0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f,-0.5f, 0.5f,  0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f, -0.5f,-0.5f,-0.5f,
        -0.5f, 0.5f,-0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,  0.5f,-0.5f,-0.5f,  0.5f, 0.5f,-0.5f,  0.5f, 0.5f,-0.5f, -0.5f, 0.5f,-0.5f, -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f,  0.5f,-0.5f, 0.5f, -0.5f,-0.5f, 0.5f
    };

    glGenVertexArrays(1, &cube_vao);
    glGenBuffers(1, &cube_vbo);

    glBindVertexArray(cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    setup_cone_geometry();
    setup_circle_geometry();
    setup_platform_edge_geometry();
    
    glBindVertexArray(0);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // FOG SETUP
    glEnable(GL_FOG);
    float fogColor[4] = {0.1f, 0.1f, 0.15f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, FADE_START_DISTANCE);
    glFogf(GL_FOG_END, FADE_END_DISTANCE);

    reshape_renderer(w, h);
}

void reshape_renderer(int w, int h) {
    glViewport(0, 0, w, h);
    float aspect = (float)w / (float)h;
    mat4_perspective(projection_matrix, 45.0f * (3.14159f / 180.0f), aspect, 0.1f, 1000.0f);
}

void draw_scene(GameState* state) {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up camera (view matrix)

    // Adjust 'up' vector for different gravities
    //if (state->gravity_direction == GRAVITY_LEFT) up = (vec3){1.0f, 0.0f, 0.0f};
    //if (state->gravity_direction == GRAVITY_RIGHT) up = (vec3){-1.0f, 0.0f, 0.0f};
    //if (state->gravity_direction == GRAVITY_UP) up = (vec3){0.0f, -1.0f, 0.0f};

    vec3 eye = {state->player.position.x, state->player.position.y + 2.0f, state->player.position.z + 8.0f};
    vec3 center = {state->player.position.x, state->player.position.y, state->player.position.z - 1.0f};
    vec3 up = quat_rotate_vec3(state->camera_orientation, (vec3){0, 1, 0});

    float view_matrix[16];
    mat4_lookAt(view_matrix, eye, center, up);

    Level* level = &state->level;
    float model_matrix[16], scale_matrix[16], trans_matrix[16], final_matrix[16];

    // Draw Platforms with Neon Tube Effect
    glUseProgram(emissive_shader);
    glUniformMatrix4fv(emissive_proj_loc, 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(emissive_view_loc, 1, GL_FALSE, view_matrix);
    glUniform1f(emissive_fade_start_loc, FADE_START_DISTANCE);
    glUniform1f(emissive_fade_end_loc, FADE_END_DISTANCE);
    
    glBindVertexArray(platform_edges_vao);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for(int i=0; i < level->platform_count; ++i) {
        Platform* p = &level->platforms[i];
        mat4_translate(trans_matrix, p->position.x, p->position.y, p->position.z);
        // This scaling correctly turns the unit cube edges into a rectangular prism
        mat4_scale(scale_matrix, p->scale.x, p->scale.y, p->scale.z);
        mat4_multiply(final_matrix, trans_matrix, scale_matrix);
        glUniformMatrix4fv(emissive_model_loc, 1, GL_FALSE, final_matrix);

        glLineWidth(7.0f);
        glUniform4f(emissive_color_loc, 0.1f, 1.0f, 0.2f, 0.3f); // Green glow
        glDrawArrays(GL_LINES, 0, 24);

        glLineWidth(2.0f);
        glUniform4f(emissive_color_loc, 0.8f, 1.0f, 0.9f, 1.0f); // White-hot core
        glDrawArrays(GL_LINES, 0, 24);
    }
    glDisable(GL_BLEND);
    glLineWidth(1.0f);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reset to solid

    // Draw Spikes
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glUseProgram(emissive_shader);
    glUniformMatrix4fv(emissive_proj_loc, 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(emissive_view_loc, 1, GL_FALSE, view_matrix);
    glUniform1f(emissive_fade_start_loc, FADE_START_DISTANCE);
    glUniform1f(emissive_fade_end_loc, FADE_END_DISTANCE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBindVertexArray(cone_vao);

    for(int i=0; i < state->level.spike_count; ++i) {
        Spike* s = &state->level.spikes[i];
        
        // --- NEW: Calculate rotation matrix based on orientation ---
        float rot_matrix[16];
        quat rot_quat;
        switch(s->orientation) {
            case GRAVITY_DOWN:  rot_quat = quat_from_axis_angle((vec3){0,0,1}, PI); break;
            case GRAVITY_LEFT:  rot_quat = quat_from_axis_angle((vec3){0,0,1}, -PI/2.0f); break;
            case GRAVITY_RIGHT: rot_quat = quat_from_axis_angle((vec3){0,0,1}, PI/2.0f); break;
            case GRAVITY_UP:
            default:            rot_quat = (quat){0,0,0,1}; break;
        }
        quat_to_mat4(rot_matrix, rot_quat);

        // Build the full model matrix: translation * rotation * scale
        float trans_matrix[16], scale_matrix[16], temp_matrix[16], final_matrix[16];
        mat4_translate(trans_matrix, s->position.x, s->position.y, s->position.z);
        mat4_scale(scale_matrix, s->radius * SPIKE_OUTLINE_THICKNESS, s->height * SPIKE_OUTLINE_THICKNESS, s->radius * SPIKE_OUTLINE_THICKNESS);
        mat4_multiply(temp_matrix, rot_matrix, scale_matrix);
        mat4_multiply(final_matrix, trans_matrix, temp_matrix);
        
        glUniformMatrix4fv(emissive_model_loc, 1, GL_FALSE, final_matrix);
        glUniform4f(emissive_color_loc, 1.0f, 0.1f, 0.1f, 0.8f);

        glDrawElements(GL_LINE_LOOP, cone_indices, GL_UNSIGNED_INT, 0);
    }
    
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    
    // IMPORTANT: Reset OpenGL state for other objects
    glCullFace(GL_BACK);
    glDisable(GL_BLEND);
    glLineWidth(1.0f);

    // Draw Jump Pads
    glUseProgram(emissive_shader);
    glUniformMatrix4fv(emissive_proj_loc, 1, GL_FALSE, projection_matrix);
    glUniformMatrix4fv(emissive_view_loc, 1, GL_FALSE, view_matrix);
    glUniform1f(emissive_fade_start_loc, FADE_START_DISTANCE);
    glUniform1f(emissive_fade_end_loc, FADE_END_DISTANCE);
    glUniform4f(emissive_color_loc, 0.5f, 0.8f, 1.0f, 1.0f);
    
    glBindVertexArray(circle_vao);
    for(int i=0; i < state->level.jump_pad_count; ++i) {
        float model_matrix[16];
        mat4_translate(model_matrix, state->level.jump_pads[i].position.x, state->level.jump_pads[i].position.y, state->level.jump_pads[i].position.z);
        glUniformMatrix4fv(emissive_model_loc, 1, GL_FALSE, model_matrix);
        glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);
    }

    glBindVertexArray(cube_vao);

    // Draw Player
    if (!state->player.is_dead) {
        glDisable(GL_CULL_FACE);
        glUseProgram(player_shader);
        glUniformMatrix4fv(player_proj_loc, 1, GL_FALSE, projection_matrix);
        glUniformMatrix4fv(player_view_loc, 1, GL_FALSE, view_matrix);
        
        float trans_matrix[16], rot_matrix[16], model_matrix[16];
        mat4_translate(trans_matrix, state->player.position.x, state->player.position.y, state->player.position.z);
        quat_to_mat4(rot_matrix, state->player.orientation);
        mat4_multiply(model_matrix, trans_matrix, rot_matrix); // Combine translation and rotation
        
        glUniformMatrix4fv(player_model_loc, 1, GL_FALSE, model_matrix);
        glBindVertexArray(cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    
        // Draw Ghost Trail
        glEnable(GL_BLEND);
        glCullFace(GL_BACK);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glUseProgram(trail_shader);
        glUniformMatrix4fv(trail_proj_loc, 1, GL_FALSE, projection_matrix);
        glUniformMatrix4fv(trail_view_loc, 1, GL_FALSE, view_matrix);

        for (int i = 0; i < state->player.trail.count; ++i) {
            int index = (state->player.trail.head_index - i + TRAIL_LENGTH) % TRAIL_LENGTH;
            vec3 pos = state->player.trail.positions[index];
            float alpha = 0.5f * (1.0f - (float)i / TRAIL_LENGTH);

            mat4_translate(model_matrix, pos.x, pos.y, pos.z);
            glUniformMatrix4fv(trail_model_loc, 1, GL_FALSE, model_matrix);
            glUniform4f(trail_color_loc, 0.1f, 0.8f, 1.0f, alpha);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_BLEND);
        glBindVertexArray(0);
    }
    
    glutSwapBuffers();
}

void free_renderer_resources() {
    glDeleteProgram(player_shader);
    glDeleteProgram(trail_shader);
    glDeleteProgram(wireframe_shader);
    glDeleteProgram(spike_shader);
    glDeleteProgram(jumppad_shader);

    glDeleteVertexArrays(1, &cube_vao);
    glDeleteBuffers(1, &cube_vbo);

    glDeleteVertexArrays(1, &platform_edges_vao);
    glDeleteBuffers(1, &platform_edges_vbo);
    
    glDeleteVertexArrays(1, &circle_vao);
    glDeleteBuffers(1, &circle_vbo);

    glDeleteVertexArrays(1, &cone_vao);
    glDeleteBuffers(1, &cone_vbo);
    glDeleteBuffers(1, &cone_ebo);
}

GLuint create_shader_program(const char* vert_src, const char* frag_src) {
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_src, NULL);
    glCompileShader(vert_shader);
    GLint success;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vert_shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_src, NULL);
    glCompileShader(frag_shader);
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(frag_shader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return program;
}