typedef int V __attribute__((vector_size(16)));
typedef long (^Block)(long);
struct Header {void* isa;int flags,reserved;long (*invoke)(void*,long);void* descriptor;long capture;};
long entry(void* raw,long x){return static_cast<Header*>(raw)->capture+x;}
struct Record {Block fn;unsigned bytes;};
inline V unused(){return V{1,2,3,4};}
inline void unknown(){__builtin_audit_unavailable();}
inline void dormant(){unknown();}
template<class T> auto shape(int)->decltype(sizeof(T{1,2})){return sizeof(T{1,2});}
template<class T> auto build(Block f)->decltype((T){.fn=f,.bytes=sizeof(V)}){return (T){.fn=f,.bytes=sizeof(V)};}
template<int N> long work(Block f,long x){Record r=build<Record>(f);return r.fn(x)+r.bytes+N+shape<V>(0);}
int main(int argc,char**){Header h={0,0,0,entry,0,7};Block f=(Block)&h;return work<3>(f,argc)!=43;}
