struct E {};
struct H { [[no_unique_address]] E e; };
struct A { int x; [[no_unique_address]] H h; char y; };
struct B { int x; [[no_unique_address]] E a,b; };
struct C { [[no_unique_address]] E e; int x; C(int n):e(),x(n){} };
struct D { int x; [[no_unique_address]] E e; D(int n):x(n),e(){} };
A global{0x12345678,{},7};
static_assert(sizeof(A)==8 && sizeof(B)==8 && sizeof(C)==4 && sizeof(D)==4,"empty overlap layout");
static_assert(__is_empty(H),"recursively empty");
int main(){A a{0x12345678,{},7}; A b=a; A c{}; c=b; B repeated{2,{},{}};
 C x(3);D y(0x12345678);
 return a.x!=0x12345678 || b.x!=a.x || c.x!=a.x || c.y!=7 || global.x!=a.x || global.y!=7 ||
  (void*)&a.x!=(void*)&a.h || &repeated.a==&repeated.b || x.x!=3 || y.x!=a.x; }
