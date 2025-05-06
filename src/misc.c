#ifndef MISC
#define MISC

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

#endif
