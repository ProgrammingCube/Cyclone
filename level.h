#ifndef LEVEL_H
#define LEVEL_H

#include "types.h" // Use the new types header instead of gamestate.h
#include "math_utils.h"
#include <stdbool.h>

#define MAX_LEVEL_OBJECTS 512
#define MAX_FILENAME_LEN 128

// A rectangular prism platform
typedef struct {
    vec3 position;
    vec3 scale;
} Platform;

// A spike obstacle
typedef struct {
    vec3 position;
    float radius;
    float height;
    GravityDirection orientation; // UP, DOWN, LEFT, RIGHT
} Spike;

// A jump pad
typedef struct {
    vec3 position;
} JumpPad;

// A trigger to change gravity
typedef struct {
    float trigger_z;
    GravityDirection new_gravity;
} GravityChanger;

typedef enum {
    ROTATION_ABSOLUTE,
    ROTATION_RELATIVE
} RotationType;

// A trigger to move the camera to an ABSOLUTE orientation
typedef struct {
    float trigger_z;
    quat target_orientation;
    float duration;
} RotationChanger;

// A trigger to perform a continuous RELATIVE camera spin
typedef struct {
    float trigger_z;
    vec3 axis;
    float speed; // calculated from total angle and duration
    float duration;
} CameraSpinChanger;

typedef struct {
    float trigger_z;
    vec3 new_axis;
    float new_speed; // in radians/sec
} PlayerSpinChanger;


// A structure to hold the entire level's data
typedef struct {
    char music_file[MAX_FILENAME_LEN];
    float initial_speed_multiplier;

    vec3 start_position;
    bool start_position_set;

    Platform platforms[MAX_LEVEL_OBJECTS];
    int platform_count;

    Spike spikes[MAX_LEVEL_OBJECTS];
    int spike_count;

    JumpPad jump_pads[MAX_LEVEL_OBJECTS];
    int jump_pad_count;

    GravityChanger gravity_changers[MAX_LEVEL_OBJECTS];
    int gravity_changer_count;
    int next_gravity_changer_index;

    PlayerSpinChanger player_spin_changers[MAX_LEVEL_OBJECTS];
    int player_spin_changer_count;
    int next_player_spin_changer_index;

    RotationChanger rotation_changers[MAX_LEVEL_OBJECTS];
    int rotation_changer_count;
    int next_rotation_changer_index;

    CameraSpinChanger camera_spin_changers[MAX_LEVEL_OBJECTS];
    int camera_spin_changer_count;
    int next_camera_spin_changer_index;

} Level;

bool load_level(const char* filename, Level* level);

#endif // LEVEL_H