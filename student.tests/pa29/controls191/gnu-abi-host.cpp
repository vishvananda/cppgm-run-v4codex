#include "gnu-abi.h"
extern "C" V host_vector(V x){return x;}
int main(){V x{1,2,3,4,5,6,7,8};auto y=native_vector(x);for(int i=0;i<8;++i)if(x[i]!=y[i])return 1;return !native_check();}
