#include "level.h"
#include "math_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PI 3.1415926535f

bool load_level(const char* filename, Level* level) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR: Could not open level file: %s\n", filename);
        return false;
    }

    memset(level, 0, sizeof(Level));
    level->initial_speed_multiplier = 1.0f;

    char line[256];
    quat current_rotation = {0, 0, 0, 1};
    level->start_position_set = false; // Initialize to false
    level->next_player_spin_changer_index = 0;

    while (fgets(line, sizeof(line), file)) {
        char type[32];
        if (sscanf(line, "%s", type) != 1) continue;

        if (strcmp(type, "music") == 0) {
            sscanf(line, "music %s", level->music_file);
        } else if (strcmp(type, "speed") == 0) {
            sscanf(line, "speed %f", &level->initial_speed_multiplier);
        }
        else if (strcmp(type, "player_spin") == 0) {
            if (level->player_spin_changer_count < MAX_LEVEL_OBJECTS) {
                PlayerSpinChanger* psc = &level->player_spin_changers[level->player_spin_changer_count];
                float speed_deg;
                sscanf(line, "player_spin %f %f %f %f %f",
                       &psc->trigger_z,
                       &psc->new_axis.x,
                       &psc->new_axis.y,
                       &psc->new_axis.z,
                       &speed_deg);
                // Convert from degrees/sec to radians/sec
                psc->new_speed = speed_deg * (PI / 180.0f);
                level->player_spin_changer_count++;
            }
        } else if (strcmp(type, "platform") == 0) {
            if (level->platform_count < MAX_LEVEL_OBJECTS) {
                Platform* p = &level->platforms[level->platform_count];
                sscanf(line, "platform %f %f %f %f %f %f",
                       &p->position.x, &p->position.y, &p->position.z,
                       &p->scale.x, &p->scale.y, &p->scale.z);
                level->platform_count++;
            }
        } else if (strcmp(type, "spike") == 0) {
            if (level->spike_count < MAX_LEVEL_OBJECTS) {
                Spike* s = &level->spikes[level->spike_count];
                char dir_str[16] = "UP"; // Default to UP if not specified
                
                sscanf(line, "spike %f %f %f %s",
                       &s->position.x, &s->position.y, &s->position.z, dir_str);

                if (strcmp(dir_str, "DOWN") == 0) s->orientation = GRAVITY_DOWN;
                else if (strcmp(dir_str, "LEFT") == 0) s->orientation = GRAVITY_LEFT;
                else if (strcmp(dir_str, "RIGHT") == 0) s->orientation = GRAVITY_RIGHT;
                else s->orientation = GRAVITY_UP;
                
                s->radius = 0.375f;
                s->height = 2.0f;
                level->spike_count++;
            }
        } else if (strcmp(type, "jumppad") == 0) {
            if (level->jump_pad_count < MAX_LEVEL_OBJECTS) {
                JumpPad* j = &level->jump_pads[level->jump_pad_count];
                sscanf(line, "jumppad %f %f %f", &j->position.x, &j->position.y, &j->position.z);
                level->jump_pad_count++;
            }
        } else if (strcmp(type, "rotate") == 0) { // This handles ABSOLUTE rotations
            if (level->rotation_changer_count < MAX_LEVEL_OBJECTS) {
                RotationChanger* r = &level->rotation_changers[level->rotation_changer_count];
                vec3 axis;
                float angle_deg;
                sscanf(line, "rotate %f %f %f %f %f %f",
                       &r->trigger_z, &axis.x, &axis.y, &axis.z, &angle_deg, &r->duration);
                r->target_orientation = quat_from_axis_angle(axis, angle_deg * (PI / 180.0f));
                level->rotation_changer_count++;
            }
        } else if (strcmp(type, "camera_spin") == 0) { // This handles RELATIVE spins
            if (level->camera_spin_changer_count < MAX_LEVEL_OBJECTS) {
                CameraSpinChanger* csc = &level->camera_spin_changers[level->camera_spin_changer_count];
                float total_angle_deg;
                sscanf(line, "camera_spin %f %f %f %f %f %f",
                       &csc->trigger_z, &csc->axis.x, &csc->axis.y, &csc->axis.z,
                       &total_angle_deg, &csc->duration);
                // Calculate speed = distance / time
                csc->speed = (total_angle_deg * (PI / 180.0f)) / csc->duration;
                level->camera_spin_changer_count++;
            }
        }
        else if (strcmp(type, "gravity") == 0) {
            if (level->gravity_changer_count < MAX_LEVEL_OBJECTS) {
                GravityChanger* g = &level->gravity_changers[level->gravity_changer_count];
                char dir_str[16];
                sscanf(line, "gravity %f %s", &g->trigger_z, dir_str);
                if (strcmp(dir_str, "UP") == 0) g->new_gravity = GRAVITY_UP;
                else if (strcmp(dir_str, "LEFT") == 0) g->new_gravity = GRAVITY_LEFT;
                else if (strcmp(dir_str, "RIGHT") == 0) g->new_gravity = GRAVITY_RIGHT;
                else g->new_gravity = GRAVITY_DOWN;
                level->gravity_changer_count++;
            }
        }
        else if (strcmp(type, "startpos") == 0) {
            sscanf(line, "startpos %f %f %f", 
                &level->start_position.x, 
                &level->start_position.y, 
                &level->start_position.z);
            level->start_position_set = true;
        
        }
    }

    fclose(file);
    printf("Level '%s' loaded successfully.\n", filename);
    printf("  Platforms: %d, Spikes: %d, JumpPads: %d, Rotations: %d, GravityChanges: %d\n",
           level->platform_count, level->spike_count, level->jump_pad_count, level->rotation_changer_count, level->gravity_changer_count);
    return true;
}