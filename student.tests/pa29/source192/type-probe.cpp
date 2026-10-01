#include <cstdio>
void f32(_Float32){} void f64(_Float64){} void f32x(_Float32x){} void f64x(_Float64x){} void f128(_Float128){} void f80(__float80){} void q(__float128){}
int main(){std::printf("f32=%d f64=%d f32x=%d f64x=%d f128=%d f80=%d\n",__is_same(_Float32,float),__is_same(_Float64,double),__is_same(_Float32x,double),__is_same(_Float64x,long double),__is_same(_Float128,__float128),__is_same(__float80,long double));}
