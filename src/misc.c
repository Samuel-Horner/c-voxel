#ifndef MISC
#define MISC

// https://blog.pkh.me/p/36-figuring-out-round%2C-floor-and-ceil-with-integer-division.html
int divFloor(int a, int b) { return a/b - (a%b!=0 && (a^b)<0); }

#endif
