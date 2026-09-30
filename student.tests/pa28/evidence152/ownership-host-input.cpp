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

#include "lazy152.h"
template<class T> Deferred152<T>::~Deferred152() {}
template<class T> Level152<T>::~Level152() {}
template class Deferred152<Leaf152>;
template class Level152<Leaf152>;
int cross152(Entry152* p){return dynamic_cast<Side152*>(p)!=0;}
