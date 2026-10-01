#if !__has_builtin(__builtin_invoke)
#error missing probe
#endif
struct Pad { long pad; };
struct Base { int x; int add(int y) const {return x+y;} virtual int virt(int y) {return x-y;} };
struct Derived : Pad, Base { int virt(int y) {return x+y+10;} };
int reads, calls;
struct Pointer { Derived* p; Derived& operator*() const {++reads; return *p;} };
struct ViaConversion { Derived* p; operator Derived*() const {++reads; return p;} };
Derived& get(Derived& d) {++calls; return d;}
int argument() {++calls; return 3;}
int main() {
 Derived d; d.x=8;
 int Base::*data=&Base::x;
 int (Base::*member)(int) const=&Base::add;
 int (Base::*virt)(int)=&Base::virt;
 if (::__builtin_invoke(member,get(d),argument())!=11 || calls!=2) return 1;
 if (__builtin_invoke(member,&d,4)!=12) return 2;
 if (__builtin_invoke(virt,Pointer{&d},4)!=22 || reads!=1) return 3;
 __builtin_invoke(data,Pointer{&d})=9;
 if(d.x!=9 || reads!=2) return 4;
 if(__builtin_invoke(member,ViaConversion{&d},1)!=10 || reads!=3) return 5;
 int Derived::*adjusted=data;
 __builtin_invoke(adjusted,d)=17;
 if(d.x!=17) return 6;
 return 0;
}
