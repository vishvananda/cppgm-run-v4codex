#include "inline-eh.h"
int from_b(int);
int source_value(int n) { return n * 2; }
int main() { return checked_value(5) == 13 && from_b(7) == 17 ? 0 : 1; }
