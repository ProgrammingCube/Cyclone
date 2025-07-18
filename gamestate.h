#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "types.h"
#include "level.h" // This is now safe to include

// Struct for the player's ghost trail
#define TRAIL_LENGTH 15
typedef struct {
    vec3 positions[TRAIL_LENGTH];
    int head_index;
    int count;
} GhostTrail;

// Struct for the player state
typedef struct {
    vec3 position;
    vec3 velocity;
    bool keys_pressed[256];
    bool is_grounded;
    bool is_dead;
    GhostTrail trail;

    quat orientation;
    vec3 rotation_axis;
    float rotation_speed;
} Player;

// Main struct for the entire game state
typedef struct {
    Player player;
    Level level;
    GravityDirection player_gravity_direction; // This is JUST for physics
    float speed_multiplier;
    int screen_width;
    int screen_height;
    bool game_over;

    quat camera_orientation;
    quat source_orientation;
    quat target_orientation;
    float rotation_time;
    float rotation_duration;
    bool is_rotating;

    // State for continuous 'camera_spin' command
    bool is_spinning;
    vec3 camera_spin_axis;
    float camera_spin_speed;
    float camera_spin_timer;

} GameState;

#endif // GAMESTATE_H