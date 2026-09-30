#include "attributes151.h"
Record::Record(int n) : value(n) {}
Record::~Record() {}
int Record::read() const { return value; }
int tagged(int n) { return n + 2; }
int versioned = 7;
int observed;
__attribute__((pure)) int reader() { return observed; }
int strongest(int) __attribute__((pure));
int strongest(int n) __attribute__((const)) { return n * n; }
int untouched(int n) { observed = n; return observed; }
int use_effects(int n) { return identity(n) + strongest(n) + reader() + untouched(n); }
template<class T> T later(T);
int demand_early() { return later(7); }
template<class T> __attribute__((const)) T later(T n) { return n; }
