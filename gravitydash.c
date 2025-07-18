#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "gamestate.h"
#include "renderer.h"
#include "player_controller.h"
#include "audio.h" // Include audio
#include "level.h" // Include level

// Globals
static GameState game_state;
static int last_time = 0;

// Function Prototypes
void init_game();
void render();
void update(int value);
void keyboard_down(unsigned char key, int x, int y);
void keyboard_up(unsigned char key, int x, int y);
void reshape(int w, int h);
void on_close();


void init_game() {
    game_state.screen_width = 1280;
    game_state.screen_height = 720;
    game_state.player_gravity_direction = GRAVITY_DOWN;
    game_state.camera_orientation = (quat){0, 0, 0, 1}; // Start with no rotation
    game_state.source_orientation = (quat){0, 0, 0, 1};
    game_state.target_orientation = (quat){0, 0, 0, 1};
    game_state.is_rotating = false;

    // Load the level script
    if (!load_level("level1.level", &game_state.level)) {
        // Handle error, maybe load a default empty level
    }
    game_state.speed_multiplier = game_state.level.initial_speed_multiplier;

    vec3 start_pos = {0.0f, 5.0f, 0.0f}; // A sensible default
    if (game_state.level.start_position_set) {
        start_pos = game_state.level.start_position; // Override with data from level file
    }

    // 2. Initialize the player, passing in the correct starting position.
    // This is now the single point of truth for setting the player's initial state.
    init_player(&game_state.player, start_pos);

    //game_state.player.rotation_axis = game_state.level.player_rotation_axis;
    //game_state.player.rotation_speed = game_state.level.player_rotation_speed;

    game_state.camera_orientation = (quat){0, 0, 0, 1}; // Identity quaternion (no rotation)
    game_state.source_orientation = (quat){0, 0, 0, 1};
    game_state.target_orientation = (quat){0, 0, 0, 1};
    game_state.is_rotating = false;
    game_state.rotation_time = 0.0f;
    game_state.rotation_duration = 0.0f;
    game_state.is_rotating = false;
    game_state.is_spinning = false;
    
    last_time = glutGet(GLUT_ELAPSED_TIME);
    
    init_audio();
    play_music(game_state.level.music_file);

    last_time = glutGet(GLUT_ELAPSED_TIME);
}

void render() {
    draw_scene(&game_state);
}

void update(int value) {
    int current_time = glutGet(GLUT_ELAPSED_TIME);
    float delta_time = (current_time - last_time) / 1000.0f;
    if (delta_time > 0.1f) delta_time = 0.1f; // Clamp delta_time
    last_time = current_time;

    if (game_state.is_spinning) {
        game_state.camera_spin_timer -= delta_time;
        
        // Check if the spin is over
        if (game_state.camera_spin_timer <= 0.0f) {
            game_state.is_spinning = false;
            // This corrects any floating point or off-by-one-frame errors.
            game_state.camera_orientation = game_state.target_orientation;

        } else {
            // While the spin is in progress, apply rotation frame-by-frame
            float delta_angle = game_state.camera_spin_speed * delta_time;
            quat delta_rotation = quat_from_axis_angle(game_state.camera_spin_axis, delta_angle);
            game_state.camera_orientation = quat_multiply(delta_rotation, game_state.camera_orientation);
        }
    } else if (game_state.is_rotating) { // Only do this if not spinning
        game_state.rotation_time += delta_time;
        float t = game_state.rotation_time / game_state.rotation_duration;
        if (t >= 1.0f) {
            t = 1.0f;
            game_state.is_rotating = false;
        }
        game_state.camera_orientation = quat_slerp(game_state.source_orientation, game_state.target_orientation, t);
    }

    update_player(&game_state, delta_time);
    
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // Aim for ~60 FPS
}

void keyboard_down(unsigned char key, int x, int y) {
    // Exit on ESC
    if (key == 27) {
        glutLeaveMainLoop();
        return;
    }
    if (key < 256) {
        game_state.player.keys_pressed[key] = true;
    }
}

void keyboard_up(unsigned char key, int x, int y) {
    if (key < 256) {
        game_state.player.keys_pressed[key] = false;
    }
}

void reshape(int w, int h) {
    game_state.screen_width = w;
    game_state.screen_height = h > 0 ? h : 1;
    reshape_renderer(w, h);
}

void on_close() {
    free_renderer_resources();
    shutdown_audio(); // Shutdown audio system
    printf("Renderer resources freed. Exiting.\n");
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition( 1000, 1200 );
    glutInitContextVersion(2, 1);
    glutCreateWindow("GravityDash3D");

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    printf("Using GLEW %s\n", glewGetString(GLEW_VERSION));
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    init_game();
    init_renderer(game_state.screen_width, game_state.screen_height);

    glutDisplayFunc(render);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, update, 0);
    glutKeyboardFunc(keyboard_down);
    glutKeyboardUpFunc(keyboard_up);
    glutIgnoreKeyRepeat(1);
    glutCloseFunc(on_close);

    glutMainLoop();
    
    return 0;
}