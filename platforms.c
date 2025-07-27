#include "platforms.h"

int initPlatforms( GameState* gameState )
{
    for ( int i = 0; i < NUM_PLATFORMS; ++i )
    {
        Object _platform = createObject( TYPE_PLATFORM );
        setupGeometry( &_platform );
        Vector_Push( &gameState->platforms, &_platform );
    }
    return 0;
}