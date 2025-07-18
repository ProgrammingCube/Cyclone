#include "player_controller.h"
#include "audio.h"
#include "math_utils.h"
#include <string.h>
#include <math.h>
#include <float.h>

#define PI 3.1415926535f

// --- Player Physics Constants ---
const float PLAYER_SIZE = 0.5f;
const float PLAYER_SLIDE_ACCEL = 50.0f; // Acceleration force for sliding
const float PLAYER_SLIDE_SPEED = 15.0f;
const float PLAYER_JUMP_FORCE = 12.0f;
const float FORWARD_SPEED = 20.0f;
const float GRAVITY_FORCE = -25.0f;
const float JUMP_PAD_BOUNCE_FORCE = 20.0f;

const float FRICTION_DAMPING = 0.95f;    // Friction for sliding. Lower is stronger.

bool check_cone_collision(vec3 player_pos, float player_size, Spike spike) {
    // 1. Get the player's position relative to the spike's origin
    vec3 relative_pos = {
        player_pos.x - spike.position.x,
        player_pos.y - spike.position.y,
        player_pos.z - spike.position.z
    };

    // 2. Create the INVERSE rotation of the spike
    quat inverse_rot;
    switch(spike.orientation) {
        case GRAVITY_DOWN:  inverse_rot = quat_from_axis_angle((vec3){0,0,1}, -PI); break; // 180 deg
        case GRAVITY_LEFT:  inverse_rot = quat_from_axis_angle((vec3){0,0,1}, PI/2.0f); break;  // -90 deg
        case GRAVITY_RIGHT: inverse_rot = quat_from_axis_angle((vec3){0,0,1}, -PI/2.0f); break; // 90 deg
        case GRAVITY_UP:
        default:            inverse_rot = (quat){0,0,0,1}; break; // No rotation
    }

    // 3. Rotate the player's relative position into the spike's "local space"
    vec3 local_pos = quat_rotate_vec3(inverse_rot, relative_pos);

    // 4. Perform the simple, original collision check in local space
    if (local_pos.y > spike.height || local_pos.y < 0) {
        return false; // Player is above or below the cone
    }
    
    float horizontal_dist_sq = local_pos.x * local_pos.x + local_pos.z * local_pos.z;
    float cone_radius_at_height = spike.radius * (1.0f - (local_pos.y / spike.height));
    float total_radius = player_size + cone_radius_at_height;
    
    return horizontal_dist_sq < total_radius * total_radius;
}


void init_player(Player* player, vec3 start_pos) {
    memset(player, 0, sizeof(Player));
    //player->position.y = 5.0f; // Start high up to fall onto the first platform
    player->position = start_pos;
    player->is_grounded = false;
    player->is_dead = false;

    player->orientation = (quat){0.0f, 0.0f, 0.0f, 1.0f}; // Identity quaternion
    player->rotation_axis = (vec3){0.0f, 0.0f, 0.0f};
    player->rotation_speed = 0.0f;
    
    player->trail.count = 0;
    player->trail.head_index = 0;
    memset(player->keys_pressed, 0, sizeof(player->keys_pressed));
}

void update_player(GameState* state, float delta_time) {
    if (state->player.is_dead) return;

    Player* p = &state->player;
    Level* level = &state->level;
    
    // 1. Determine Physics & Control Vectors
    vec3 gravity_up = {0, 1, 0};
    vec3 control_right = {1, 0, 0};
    switch (state->player_gravity_direction) {
        case GRAVITY_DOWN:  gravity_up = (vec3){0, 1, 0}; control_right = (vec3){1, 0, 0}; break;
        case GRAVITY_UP:    gravity_up = (vec3){0,-1, 0}; control_right = (vec3){1, 0, 0}; break;
        case GRAVITY_LEFT:  gravity_up = (vec3){1, 0, 0}; control_right = (vec3){0, 1, 0}; break;
        case GRAVITY_RIGHT: gravity_up = (vec3){-1,0, 0}; control_right = (vec3){0,-1, 0}; break;
    }
    
    // --- KEY CHANGE: Acceleration and Drag Model ---

    // 2. Apply Forces
    // Apply gravity
    p->velocity.x += gravity_up.x * GRAVITY_FORCE * delta_time;
    p->velocity.y += gravity_up.y * GRAVITY_FORCE * delta_time;

    // Apply jump force
    if (p->keys_pressed[' '] && p->is_grounded) {
        p->is_grounded = false;
        p->velocity.x += gravity_up.x * PLAYER_JUMP_FORCE;
        p->velocity.y += gravity_up.y * PLAYER_JUMP_FORCE;
        play_sound_effect("jump");
    }

    // Apply sliding acceleration from player input
    float slide_input = (p->keys_pressed['d'] || p->keys_pressed['D']) - (p->keys_pressed['a'] || p->keys_pressed['A']);
    p->velocity.x += control_right.x * slide_input * PLAYER_SLIDE_ACCEL * delta_time;
    p->velocity.y += control_right.y * slide_input * PLAYER_SLIDE_ACCEL * delta_time;

    // 3. Apply Damping and Set Final Velocity
    // Decompose velocity into vertical and sliding components
    float vertical_speed = p->velocity.x * gravity_up.x + p->velocity.y * gravity_up.y;
    vec3 vertical_velocity = {gravity_up.x * vertical_speed, gravity_up.y * vertical_speed, 0};
    vec3 slide_velocity = {p->velocity.x - vertical_velocity.x, p->velocity.y - vertical_velocity.y, 0};

    // Apply friction damping ONLY to the slide velocity
    slide_velocity.x *= FRICTION_DAMPING;
    slide_velocity.y *= FRICTION_DAMPING;

    // Recombine and set final velocity for this frame
    p->velocity.x = vertical_velocity.x + slide_velocity.x;
    p->velocity.y = vertical_velocity.y + slide_velocity.y;
    p->velocity.z = -FORWARD_SPEED * state->speed_multiplier;

    // 4. Update Position & Resolve Collisions
    p->position.x += p->velocity.x * delta_time;
    p->position.y += p->velocity.y * delta_time;
    p->position.z += p->velocity.z * delta_time;
    p->is_grounded = false; 
    
    p->is_grounded = false; // Assume not grounded until a valid collision proves otherwise
    
    for (int i=0; i < level->platform_count; ++i) {
        Platform* plat = &level->platforms[i];
        vec3 plat_min = {plat->position.x - plat->scale.x/2, plat->position.y - plat->scale.y/2, plat->position.z - plat->scale.z/2};
        vec3 plat_max = {plat->position.x + plat->scale.x/2, plat->position.y + plat->scale.y/2, plat->position.z + plat->scale.z/2};
        vec3 player_min = {p->position.x - PLAYER_SIZE, p->position.y - PLAYER_SIZE, p->position.z - PLAYER_SIZE};
        vec3 player_max = {p->position.x + PLAYER_SIZE, p->position.y + PLAYER_SIZE, p->position.z + PLAYER_SIZE};

        if(player_max.x > plat_min.x && player_min.x < plat_max.x &&
           player_max.y > plat_min.y && player_min.y < plat_max.y &&
           player_max.z > plat_min.z && player_min.z < plat_max.z)
        {
            float mtv_x = (player_max.x - plat_min.x < plat_max.x - player_min.x) ? -(player_max.x - plat_min.x) : (plat_max.x - player_min.x);
            float mtv_y = (player_max.y - plat_min.y < plat_max.y - player_min.y) ? -(player_max.y - plat_min.y) : (plat_max.y - player_min.y);
            float mtv_z = (player_max.z - plat_min.z < plat_max.z - player_min.z) ? -(player_max.z - plat_min.z) : (plat_max.z - player_min.z);

            vec3 normal = {0};
            if(fabs(mtv_x) < fabs(mtv_y) && fabs(mtv_x) < fabs(mtv_z)) {
                p->position.x += mtv_x; normal.x = (mtv_x > 0) ? 1 : -1;
            } else if (fabs(mtv_y) < fabs(mtv_z)) {
                p->position.y += mtv_y; normal.y = (mtv_y > 0) ? 1 : -1;
            } else {
                p->position.z += mtv_z; normal.z = (mtv_z > 0) ? 1 : -1;
            }

            float ground_alignment = normal.x * gravity_up.x + normal.y * gravity_up.y;
            if(ground_alignment > 0.7f) {
                p->is_grounded = true;
                float vel_into_ground = p->velocity.x * normal.x + p->velocity.y * normal.y;
                if (vel_into_ground < 0) {
                    p->velocity.x -= vel_into_ground * normal.x;
                    p->velocity.y -= vel_into_ground * normal.y;
                }
            }
        }
    }
    
    // 5. Check for Object Interactions
    // Spike collisions
    for (int i = 0; i < state->level.spike_count; ++i) {
        if (check_cone_collision(p->position, PLAYER_SIZE, state->level.spikes[i])) {
            p->is_dead = true;
            play_sound_effect("crash");
            printf("Game Over: Hit a spike!\n");
            return; // Stop updating if dead
        }
    }

    // Jump pad collisions
    for(int i = 0; i < state->level.jump_pad_count; ++i) {
        float dx = p->position.x - state->level.jump_pads[i].position.x;
        float dy = p->position.y - state->level.jump_pads[i].position.y;
        float dz = p->position.z - state->level.jump_pads[i].position.z;
        if(dx*dx + dy*dy + dz*dz < (1.0f + PLAYER_SIZE)*(1.0f + PLAYER_SIZE)) {
            
            float fall_velocity_component = p->velocity.x * gravity_up.x + p->velocity.y * gravity_up.y;
            if (fall_velocity_component < 0) { // Check if moving TOWARDS the bounce surface
                // Set velocity directly, overriding current fall speed
                p->velocity.x = gravity_up.x * JUMP_PAD_BOUNCE_FORCE;
                p->velocity.y = gravity_up.y * JUMP_PAD_BOUNCE_FORCE;
                p->is_grounded = false;
                play_sound_effect("bounce");
            }
        }
    }

    // 6. Check for Scripted Triggers

    // Rotation Triggers (Camera Only)
    if (!state->is_rotating && !state->is_spinning) { // Only trigger if camera is idle
        int next_r_idx = level->next_rotation_changer_index;
        if (next_r_idx < level->rotation_changer_count && p->position.z < level->rotation_changers[next_r_idx].trigger_z) {
            state->is_rotating = true;
            state->rotation_time = 0.0f;
            RotationChanger* r = &level->rotation_changers[next_r_idx];
            state->source_orientation = state->camera_orientation;
            state->target_orientation = r->target_orientation;
            state->rotation_duration = r->duration;
            level->next_rotation_changer_index++;
        }
    }

    // Check for Relative Spin Triggers
    if (!state->is_rotating && !state->is_spinning) { // Only trigger if camera is idle
        int next_csc_idx = level->next_camera_spin_changer_index;
        if (next_csc_idx < level->camera_spin_changer_count && p->position.z < level->camera_spin_changers[next_csc_idx].trigger_z) {
            CameraSpinChanger* csc = &level->camera_spin_changers[next_csc_idx];
            
            // Set up the state for the continuous spin
            state->is_spinning = true;
            state->camera_spin_axis = csc->axis;
            state->camera_spin_speed = csc->speed;
            state->camera_spin_timer = csc->duration;

            // 1. Calculate the total rotation of the entire spin.
            float total_angle = csc->speed * csc->duration;
            quat total_rotation = quat_from_axis_angle(csc->axis, total_angle);
            
            // 2. Apply it to the current orientation to get the final target.
            state->target_orientation = quat_multiply(total_rotation, state->camera_orientation);
            
            level->next_camera_spin_changer_index++;
        }
    }

    // check player spin triggers
    int next_psc_idx = level->next_player_spin_changer_index;
    if (next_psc_idx < level->player_spin_changer_count && p->position.z < level->player_spin_changers[next_psc_idx].trigger_z) {
        PlayerSpinChanger* psc = &level->player_spin_changers[next_psc_idx];
        p->rotation_axis = psc->new_axis;
        p->rotation_speed = psc->new_speed;
        level->next_player_spin_changer_index++;
    }

    // update player cube rotation
    if (p->rotation_speed != 0.0f) {
        float delta_angle = p->rotation_speed * delta_time;
        quat delta_rotation = quat_from_axis_angle(p->rotation_axis, delta_angle);
        p->orientation = quat_multiply(delta_rotation, p->orientation);
    }

    // Gravity Triggers (Player Physics Only)
    int next_g_idx = level->next_gravity_changer_index;
    if (next_g_idx < level->gravity_changer_count && p->position.z < level->gravity_changers[next_g_idx].trigger_z) {
        state->player_gravity_direction = level->gravity_changers[next_g_idx].new_gravity;
        level->next_gravity_changer_index++;
    }

    // 7. Update Ghost Trail
    p->trail.head_index = (p->trail.head_index + 1) % TRAIL_LENGTH;
    p->trail.positions[p->trail.head_index] = p->position;
    if (p->trail.count < TRAIL_LENGTH) {
        p->trail.count++;
    }
}