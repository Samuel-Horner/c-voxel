#ifndef VECTOR
#define VECTOR

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define GROWTH_FACTOR 2

// Some general notes:
// NEVER let the vector go to 0 in capacity, things will get very ugly very quickly. As long as a vector is not initialised to have 0 capacity, this shouldnt occur.
// Since we are using void pointers, make sure the item size is correct, otherwise you will get segfaults, which no one likes.

typedef struct Vector {
    void *vals;
    size_t item_size;
    size_t size;
    size_t capacity;
} Vector;

void* vectorAllocate(Vector *vector) {
    return malloc(vector->capacity * vector->item_size);
}

int vectorGrow(Vector *vector) {
    vector->capacity *= GROWTH_FACTOR;
    void *new_vals = vectorAllocate(vector);
    if (new_vals == NULL) { 
        vector->capacity /= GROWTH_FACTOR;
        return 0; 
    }

    memcpy(new_vals, vector->vals, vector->size * vector->item_size);
    
    free(vector->vals);
    vector->vals = new_vals;

    return 1;
}

int vectorShrink(Vector *vector) {
    vector->capacity /= GROWTH_FACTOR;
    void *new_vals = vectorAllocate(vector);
    if (new_vals == NULL) { 
        vector->capacity *= GROWTH_FACTOR;
        return 0; 
    }
   
    memcpy(new_vals, vector->vals, vector->size * vector->item_size);
    
    free(vector->vals);
    vector->vals = new_vals;

    return 1;
}

void *vectorIndex(Vector *vector, size_t index) {
    if (index >= vector->capacity) { return NULL; }
    return vector->vals + index * vector->item_size;
}

int vectorPush(Vector *vector, void *item) {
    if (vector->size + 1 > vector->capacity) {
        if (!vectorGrow(vector)) { return 0; }
    }

    memcpy(vectorIndex(vector, vector->size), item, vector->item_size);
    vector->size++;

    return 1;
}

int vectorPushArray(Vector *vector, void *array, size_t array_size) {
    vector->size += array_size;

    if (vector->size > vector->capacity) {
        if (!vectorGrow(vector)) { vector->size -= array_size; return 0; }
    }

    memcpy(vectorIndex(vector, vector->size - array_size), array, vector->item_size * array_size);
    return 1;
}

// int vectorPopIndex(Vector *vector, void *dest, size_t index) {
//     if (vector->size == 0) { return 0; }
//     void *item = vectorIndex(vector, index);
//     void *vals = vector->vals;
// 
//     vector->vals = malloc(vector->capacity * vector->item_size);
//     if (vector->vals == NULL) { vector->vals = vals; return 0; }
// 
//     memcpy(vector->vals, vals, index * vector->item_size);
//     memcpy(vector->vals + (index * vector->item_size), vals + ((index + 1) * vector->item_size), (vector->size - index) * vector->item_size);
// 
//     if (dest != NULL) { memcpy(dest, item, vector->item_size); }
//     free(vals);
// 
//     vector->size--;
//     
//     if (vector->size < vector->capacity / GROWTH_FACTOR) {
//         if (!vectorShrink(vector)) { return 0; }
//     }
//     
//     return 1;
// }

int vectorPop(Vector *vector, void *dest) {
    if (vector->size == 0) { return 0; }
    void *top = vectorIndex(vector, vector->size - 1);
    if (dest == NULL || top == NULL) { return 0; } 
    
    memcpy(dest, top, vector->item_size);
    vector->size--;

    if (vector->size < vector->capacity / GROWTH_FACTOR) {
        if (!vectorShrink(vector)) { return 0; }
    }

    return 1;
}

// int vectorConcat(Vector *v, Vector *dest) {
//     if (v->item_size != dest->item_size) { 
//         printf("ERROR: Attempted to concat incompatable vector types.\n");
//         return 0; 
//     }
// 
//     while (dest->capacity - dest->size < v->size) {
//         if (!vectorGrow(dest)) { 
//             printf("ERROR: Cannot grow destination vector.\n");
//             return 0; 
//         }
//     }
// 
//     memcpy(dest->vals + (dest->size * dest->item_size), v->vals, v->item_size *  v->size);
//     dest->size += v->size;
// 
//     return 1;
// }

void freeVector(Vector *vector) {
    if (vector->vals == NULL) { return; }
    free(vector->vals);
    vector->vals = NULL;
}

Vector vectorInit(size_t item_size, size_t initial_capacity) {
    Vector vector;
    vector.capacity = initial_capacity;
    vector.item_size = item_size;
    vector.size = 0;
    vector.vals = vectorAllocate(&vector);
    if (vector.vals == NULL) { printf("ERROR: Failed to allocated space for vector\n"); }
    return vector;
}

Vector vectorFromArray(size_t item_size, size_t array_size, void *array) {
    Vector vector = vectorInit(item_size, array_size);
    if (vector.vals == NULL) { return vector; }

    memcpy(vector.vals, array, array_size * item_size);
    vector.size = array_size;

    return vector;
}

#define printVector(type, format, vector, vals_per_line) do { \
    printf("{0: "); \
    for (size_t i = 0; i < vector.size; i++) { \
        printf(format, *((type *) vectorIndex(&vector, i))); \
        if (i < vector.size - 1) { printf(", "); } \
        if ((i + 1) % vals_per_line == 0) { printf("\n %d: ", i / vals_per_line); } \
    } \
    printf("} Size: %zu, Capacity: %zu\n", vector.size, vector.capacity); \
} while(0)

#endif
