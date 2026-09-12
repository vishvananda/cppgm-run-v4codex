// Fixed scalar facts share types/conversions, while locals and effects belong
// to each instantiated function. C++11 [temp.res], [temp.dep.expr], [expr].
int touched=0;
template<class T> int fixed(int n) {
    volatile int v=n; int a=n+2; int b=3;
    int& ref=a;
    ++v; b+=a; a=b*2; ref-=1;
    int* p=&a;
    int z=static_cast<int>((n+0.5)*2);
    { int a=n-1; b+=a; }
    touched+=1;
    return ((n>0 && b>0) ? ++a : --b) + v + *p + z;
}
struct A{};
int early() { return fixed<A>(4); }
// Class completion projects a source region before later regions are parsed.
template<class T>struct Stamp{T x;}; Stamp<int> stamp;
template<class T> int later(int x){int y=x+1;return (y*=2)+x;}
template<class T> int sizes(int x) {
    int y=x;
    return sizeof(++x)!=sizeof(int) || x!=y || alignof(double)!=8 || (&x)[0]!=y;
}
template<class T> struct Outer {
    static int run(int n) { int x=n; { int y=2; x+=y; } return x*3; }
    struct Inner { static int run(int n) { int x=n+4; return x/2; } };
};
int main(){
    int a=early(),b=fixed<long>(5);
    if(a!=50 || b!=57 || touched!=2) return 1;
    if(later<char>(3)!=11 || later<A>(5)!=17) return 2;
    if(Outer<int>::run(2)!=12 || Outer<long>::run(3)!=15) return 3;
    if(Outer<int>::Inner::run(2)!=3 || Outer<long>::Inner::run(4)!=4) return 4;
    if(sizes<int>(5) || sizes<A>(7)) return 5;
    return 0;
}
