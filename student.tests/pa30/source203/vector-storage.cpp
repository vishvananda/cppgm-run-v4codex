typedef int V __attribute__((vector_size(16)));
typedef unsigned int U __attribute__((vector_size(16)));
typedef float F __attribute__((vector_size(16)));
V global = {1,2,3,4};
volatile V observed = {7,8,9,10};
int effects;
V& ref() { ++effects; return global; }
int index() { ++effects; return 2; }
V mutate() { global[0]=100; return V{4,5,6,7}; }
template<class T> auto get(T& v,int i) -> decltype(v[i]) { return v[i]; }
static_assert(__is_same(decltype(global[1]),int&),"lane lvalue");
static_assert(__is_same(decltype(observed[1]),volatile int&),"lane cv");
static_assert(__is_same(decltype(V{}[0]),int),"temporary lane");
int main(int argc,char**) {
    V x = {argc,20,30,40};
    x[argc] += 3;
    ref()[index()]++;
    if(effects!=2 || global[2]!=4 || get(x,1)!=23) return 1;
    V sum=global+mutate();
    if(sum[0]!=5 || sum[2]!=10) return 2;
    V saved=observed; observed=x;
    if(saved[3]!=10 || observed[1]!=23) return 3;
    x+=V{1,2,3,4};
    U bits=(U)x;
    F floats=__builtin_convertvector(x,F);
    if(bits[1]!=25 || floats[2]!=33.0f) return 4;
    V y=x; x[1]=99;
    return y[1]!=25 || V{5,6,7,8}[argc]!=6;
}
