int effects=0;
void* effect(){++effects;return 0;}
constexpr unsigned width(){return 8;}
template<unsigned N> struct Check { static_assert(__atomic_always_lock_free(N,0),"constant query"); };
template<unsigned N> constexpr bool supported(){return __c11_atomic_is_lock_free(N);}
static_assert(supported<8>(),"dependent query");
Check<4> check;
static_assert(noexcept(__atomic_always_lock_free(4,effect())),"unevaluated pointer");
int main(){
 bool a=__atomic_always_lock_free(width(),effect());
 bool b=__atomic_is_lock_free(4,effect());
 char c=0;void* v=&c;bool old=__atomic_test_and_set(v,2);__atomic_clear(v,3);
 return a && b && effects==1 && !old && !c?0:1;
}
