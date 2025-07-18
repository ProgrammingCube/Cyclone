#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <stdio.h>
#include "gamestate.h"

void init_player(Player* player, vec3 start_pos );
void update_player(GameState* state, float delta_time);

#endif // PLAYER_CONTROLLER_H