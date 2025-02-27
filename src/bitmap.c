#ifndef BITMAP
#define BITMAP

#include <limits.h>
#include <stdint.h>

// Referencing https://stackoverflow.com/questions/1225998/what-is-a-bitmap-in-c

typedef uint16_t word_t;
enum { BITS_PER_WORD = sizeof(word_t) * CHAR_BIT }; 

#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

void setBit(word_t *words, int n) { 
    words[WORD_OFFSET(n)] |= (1 << BIT_OFFSET(n));
}

void clearBit(word_t *words, int n) {
    words[WORD_OFFSET(n)] &= ~(1 << BIT_OFFSET(n)); 
}

int getBit(word_t *words, int n) {
    word_t bit = words[WORD_OFFSET(n)] & (1 << BIT_OFFSET(n));
    return bit != 0; 
}

#endif
