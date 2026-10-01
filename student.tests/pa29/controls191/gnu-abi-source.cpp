#include "gnu-abi.h"
extern "C" V native_vector(V x){return x;}
int native_check(){V x{1,2,3,4,5,6,7,8}; return __builtin_reduce_or(host_vector(x)!=x)==0;}
