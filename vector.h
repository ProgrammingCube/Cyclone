#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>

typedef struct sVector
{
    void* items;
    size_t item_size;
    int count;
    int capacity;
} Vector;

void Vector_Init( Vector* vec, size_t item_size );
int Vector_Free( Vector* vec );
int Vector_Push( Vector* vec, void* item );
int Vector_Pop( Vector* vec, int index );
void* Vector_Get( Vector* vec, int index );

#endif