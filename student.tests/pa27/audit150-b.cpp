#include "audit150-trace.h"
extern "C" Measure150 address150b() { return measure150<11>; }
extern "C" long entry150b(int n) { return measure150<11>(n); }
