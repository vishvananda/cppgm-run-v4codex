#include "extern.h"
extern template struct Box<int>::Inner;
int call(int x){return Box<int>::Inner::value(x);}
