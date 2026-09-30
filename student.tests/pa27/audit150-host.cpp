#include "audit150-trace.h"
int destroyed;
long imported[2] = {17,19};
thread_local int ticket = 3;
extern long* checkpoint150;
extern int aligned150;
extern "C" Measure150 address150a(), address150b(), local150();
extern "C" long entry150a(int), entry150b(int);
int main(int argc, char**) {
    if (address150a()!=address150b() || local150()(argc)!=6) return 1;
    if (entry150a(argc)!=36 || entry150b(argc)!=36 || destroyed!=2) return 2;
    if (checkpoint150!=&imported[1] || aligned150!=23 || (reinterpret_cast<unsigned long>(&aligned150)&31)) return 3;
    try { entry150a(-2); return 4; }
    catch (int n) { if (n!=-1 || destroyed!=2) return 5; }
    ticket=7;
    return entry150b(argc)==40 && destroyed==3 ? 0:6;
}
