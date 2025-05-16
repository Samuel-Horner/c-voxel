#ifndef MISC
#define MISC

#include <time.h>
#include <stdio.h>
#include <errno.h>

// https://blog.pkh.me/p/36-figuring-out-round%2C-floor-and-ceil-with-integer-division.html
int divFloor(int a, int b) { return a/b - (a%b!=0 && (a^b)<0); }

// https://stackoverflow.com/questions/11720656/modulo-operation-with-negative-numbers
// This is actually the modulo operation, not the remainder operation
int mod(int a, int b) {
    // if (a < 0) { return -((a + 1) % b); }
    // else { return (a % b); }
    int r = a % b;
    return r < 0 ? r + b : r;
}

#define max(a, b) (a > b ? a : b)

float getTimeStamp() {
    struct timespec spec;
    if (clock_gettime(CLOCK_MONOTONIC, &spec) != 0) {
        int errsv = errno;
        printf("ERROR: in clock_gettime. errno: %d\n", errno);
        return -1.;
    }

    return (float) spec.tv_sec + spec.tv_nsec / 1.0e9;
}

#endif
