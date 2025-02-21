#ifndef VECTOR
#define VECTOR

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define GROWTH_FACTOR 2

typedef struct Vector {
    void *vals;
    size_t item_size;
    size_t size;
    size_t capacity;
} Vector;

int vectorAllocate (Vector *vector) {
    void *new_vals = malloc(vector->capacity * vector->item_size);
    if (new_vals == NULL) { return 0; }

    vector->vals = new_vals;
    return 1;
}

int vectorGrow (Vector *vector) {
    void *vals = vector->vals;
    
    vector->capacity *= GROWTH_FACTOR;
    if (!vectorAllocate(vector)) {
        vector->vals = vals;
        return 0;
    }

    memcpy(vector->vals, vals, vector->size * vector->item_size);
    free(vals);
    return 1;
}

int vectorShrink (Vector *vector) {
    void *vals = vector->vals;
    
    vector->capacity /= GROWTH_FACTOR;
    if (vector->capacity == 0) {vector->capacity = 1; }
    if (!vectorAllocate(vector)) {
        vector->vals = vals;
        return 0;
    }

    memcpy(vector->vals, vals, vector->size * vector->item_size);
    free(vals);
    return 1;
}

void *vectorIndex (Vector *vector, size_t index) {
    if (index >= vector->size) { return NULL; }
    return vector->vals + index * vector->item_size;
}

int vectorPush (Vector *vector, void *item) {
    vector->size++;
    
    if (vector->size > vector->capacity) {
        if (!vectorGrow(vector)) { vector->size--; return 0; }
    }

    memcpy(vectorIndex(vector, vector->size - 1), item, vector->item_size);
    return 1;
}

int vectorPopIndex (Vector *vector, void *dest, size_t index) {
    if (vector->size == 0) { return 0; }
    void *item = vectorIndex(vector, index);
    void *vals = vector->vals;

    size_t temp = (vector->size - 1) * vector->item_size;
    vector->vals = malloc(vector->capacity * vector->item_size);
    if (vector->vals == NULL) { vector->vals = vals; return 0; }

    memcpy(vector->vals, vals, index * vector->item_size);
    memcpy(vector->vals + (index * vector->item_size), vals + ((index + 1) * vector->item_size), (vector->size - index) * vector->item_size);

    if (dest != NULL) { memcpy(dest, item, vector->item_size); }
    free(vals);

    vector->size--;
    
    if (vector->size < vector->capacity / GROWTH_FACTOR) {
        if (!vectorShrink(vector)) { return 0; }
    }
    
    return 1;
}

int vectorPop (Vector *vector, void *dest) {
    if (vector->size == 0) { return 0; }
    void *top = vectorIndex(vector, vector->size - 1);
    if (dest != NULL) { memcpy(dest, top, vector->item_size); }
    
    vector->size--;

    if (vector->size < vector->capacity / GROWTH_FACTOR) {
        if (!vectorShrink(vector)) { return 0; }
    }

    return 1;
}

int vectorConcat(Vector *v, Vector *dest) {
    if (v->item_size != dest->item_size) { 
        printf("ERROR: Attempted to concat incompatable vector types.\n");
        return 0; 
    }

    while (dest->capacity - dest->size < v->size) {
        if (!vectorGrow(dest)) { 
            printf("ERROR: Cannot grow destination vector.\n");
            return 0; 
        }
    }

    memcpy(dest->vals + (dest->size * dest->item_size), v->vals, v->item_size *  v->size);
    dest->size += v->size;

    return 1;
}

void vectorFree (Vector *vector) {
    free(vector->vals);
    // free(vector); // For some reason this causes a double free, even with the above line commeneted out?
}

Vector vectorInit (size_t item_size, size_t initial_capacity) {
    Vector vector;
    vector.capacity = initial_capacity;
    vector.item_size = item_size;
    vector.size = 0;
    if (!vectorAllocate(&vector)) { 
        printf("ERROR: Failed to allocated space for vector\n");
        exit(1);
    }
    return vector;
}

Vector vectorFromArray(size_t item_size, size_t array_size, void *array) {
    Vector vector = vectorInit(item_size, array_size);

    memcpy(vector.vals, array, array_size * item_size);
    vector.size = array_size;

    return vector;
}

#define printVector(type, format, vector) do { \
    printf("{"); \
    for (size_t i = 0; i < vector.size; i++) { \
        printf(format, *((type *) vectorIndex(&vector, i))); \
        if (i < vector.size - 1) { printf(", "); } \
    } \
    printf("} Size: %zu, Capacity: %zu\n", vector.size, vector.capacity); \
} while(0)

#endif
