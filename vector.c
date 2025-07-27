#include "vector.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int Vector_Resize( Vector* vec, int new_capacity )
{
    void* new_items = realloc( vec->items, new_capacity * vec->item_size );
    if ( new_items == NULL )
    {
        fprintf( stderr, "VECTOR: realloc failed Vector_Resize\n" );
        return -1;
    }
    vec->items = new_items;
    vec->capacity = new_capacity;
    return 0;
}

void Vector_Init( Vector* vec, size_t item_size )
{
    vec->items = NULL;
    vec->item_size = item_size;
    vec->count = 0;
    vec->capacity = 0;
}

int Vector_Free( Vector* vec )
{
    if ( !vec->items )
    {
        free( vec->items );
        vec->items = NULL;
        vec->count = 0;
        vec->capacity = 0;
    }
    else
    {
        fprintf( stderr, "VECTOR: unable to complete Vector_Free\n" );
        return -1;
    }
    return 0;
}

int Vector_Push( Vector* vec, void* item )
{
    if ( vec->count == vec->capacity )
    {
        int new_capacity = ( vec->capacity == 0 ) ? 8 : vec->capacity * 2;
        if ( Vector_Resize( vec, new_capacity ) != 0 )
        {
            return -1;
        }
    }
    char* destination = ( char* )vec->items + ( vec->count * vec->item_size );
    memcpy( destination, item, vec->item_size );
    vec->count++;
    return 0;
}

void* Vector_Get( Vector* vec, int index )
{
    if ( index >= vec->count || index < 0 )
    {
        return NULL;
    }
    return ( char* )vec->items + ( index * vec->item_size );
}