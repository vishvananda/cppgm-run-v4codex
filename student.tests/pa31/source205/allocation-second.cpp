#include <new>
int second() { int* p = new int[4]; p[3] = 11; int n = p[3]; delete[] p; return n; }
