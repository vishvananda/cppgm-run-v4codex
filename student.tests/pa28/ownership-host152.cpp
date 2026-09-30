#include "ownership152.h"
int destroyed152;
HostRoot152::HostRoot152() : value(0) {}
HostRoot152::~HostRoot152() { ++destroyed152; }
HostMiddle152::HostMiddle152() {}
HostMiddle152::~HostMiddle152() {}
HostLeaf152::HostLeaf152() {}
HostLeaf152::~HostLeaf152() {}
HostRoot152::operator bool() const { return value != 0; }
HostMiddle152& fill152(HostMiddle152& r,int value) { r.value=value; return r; }
