#include "lazy152.h"
template<class T> Deferred152<T>::~Deferred152() {}
template<class T> Level152<T>::~Level152() {}
template class Deferred152<Leaf152>;
template class Level152<Leaf152>;
int main() {
  Entry152* a=first152(); Entry152* b=second152();
  if(a == b || first152() != a || second152() != b) return 1;
  return !dynamic_cast<Side152*>(a) || !dynamic_cast<Side152*>(b);
}
