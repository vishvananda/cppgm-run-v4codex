struct Header { void* isa; int flags; int reserved; long (*invoke)(void*,long); void* descriptor; long capture; };
long implementation(void* raw,long value) { return static_cast<Header*>(raw)->capture+value; }
typedef long (^Block)(long);
Block identity(Block value) {return value;}
int fetches;
Block fetch(Block value) {++fetches; return value;}
template<class T> T invoke(T (^value)(T),T argument) {return value(argument);}
template<class B> auto generic(B value,long argument)->decltype(value(argument)) {return value(argument);}
template<class B> auto select(B value,int)->decltype(value(1L)) {return value(2L);}
long select(...){return -100;}
struct Owner {Block block; long call(long v) const { return block(v); }};
int main() {
 Header first = {0,0,0,implementation,0,10}; Header second = {0,0,0,implementation,0,20};
 Block a = reinterpret_cast<Block>(&first), b = (Block)&second;
 Block* p = &a; Block list[2] = {a,b}; Owner owner = {a};
 if(a(1)!=11 || (*p)(2)!=12 || list[1](3)!=23 || identity(a)(4)!=14)return 1;
 if(fetch(a)(5)!=15 || fetches!=1 || owner.call(6)!=16)return 2;
 if(invoke(a,7L)!=17 || generic(b,8)!=28 || select(a,0)!=12 || select(3,0)!=-100)return 3;
 if((true ? a : b)(9)!=19 || a==b || !(a!=b) || !a)return 4;
 void* opaque = a; if(opaque!=&first)return 5;
 return 0;
}
