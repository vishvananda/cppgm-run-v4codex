#include "audit150-trace.h"
long* checkpoint150 __attribute__((section(".audit150"))) = &imported[1];
alignas(32) int aligned150 __attribute__((section(".audit150"))) = 23;
static long unused150(int n) { return n+100; }
static long retained150(int n) { return n+5; }
extern "C" Measure150 address150a() { return measure150<11>; }
extern "C" Measure150 local150() { return retained150; }
extern "C" long entry150a(int n) { return measure150<11>(n); }
