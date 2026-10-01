template<class T> __attribute__((abi_tag("keep"))) int value(T x){return x+1;}
template<> int value<long>(long x){return x+2;}
int main(){return value(1)!=2 || value(1L)!=3;}
