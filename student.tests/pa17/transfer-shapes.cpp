// O0 conversion boundaries for ordinary and specialized member storage.
struct X { long value; void f(){value=7;value=0;} };
template<class T>struct Y { T value; };
int main(){X x; x.value=7; x.value=0; x.f();Y<long> y;y.value=9;return x.value+y.value-9;}
