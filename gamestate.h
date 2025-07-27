#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "player.h"
#include "vector.h"

#define MAX_PLATFORMS   50

typedef struct sGameState
{
    Object player;
    Vector platforms;
    Vector spikes;
    Vector jumppads;

    int num_platforms;

    int score;
    int level;
    bool is_running;
} GameState;

void GameState_Init( GameState* state );
void GameState_Shutdown( GameState* state );

#endif