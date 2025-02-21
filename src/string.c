#ifndef VEC_STRING
#define VEC_STRING

#include "vector.c"

#include "stdlib.h"
#include "stdio.h"

#define String Vector

#define stringFromArray(array, array_size) vectorFromArray(sizeof(char), array_size, array)

#define stringInit(initial_capacity) vectorInit(sizeof(char), initial_capacity)

#define stringConcat(s, dest) vectorConcat(s, dest)

#endif
