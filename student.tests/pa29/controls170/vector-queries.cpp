typedef int V __attribute__((vector_size(16)));
typedef float F __attribute__((ext_vector_type(3)));
static_assert(sizeof(V{1,2,3,4})==16,"vector list query");
static_assert(noexcept(V{1,2}),"vector nonthrowing query");
template<class T> auto size_of(int)->decltype(sizeof(T{1,2})){return sizeof(T{1,2});}
template<class T> auto valid(int)->decltype(T{1,2,3,4,5},int()){return 1;}
template<class T> long valid(...){return 2;}
static_assert(__is_same(decltype(valid<V>(0)),long),"excess lanes fail substitution");
template<class T> auto narrow(int)->decltype(T{1.5},int()){return 1;}
template<class T> long narrow(...){return 2;}
static_assert(__is_same(decltype(narrow<V>(0)),long),"narrowing fails substitution");
int main(){return size_of<V>(0)!=16 || size_of<F>(0)!=16 || valid<V>(0)!=2 || narrow<V>(0)!=2;}
