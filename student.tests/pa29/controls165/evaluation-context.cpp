#if !__has_builtin(__builtin_is_constant_evaluated)
#error missing probe
#endif
constexpr bool active(){return __builtin_is_constant_evaluated();}
constexpr int choose(int x){return active()?x+4:x-2;}
static_assert(active(),"constant execution");
static_assert(choose(8)==12,"constant call");
static_assert(noexcept(__builtin_is_constant_evaluated()),"nonthrowing");
static_assert(::__builtin_is_constant_evaluated(),"global intrinsic");
static_assert(__is_same(decltype(__builtin_is_constant_evaluated()),bool),"type");
template<bool B> struct Flag {static constexpr bool value=B;};
template<class T> int work(int x){static_assert(Flag<__builtin_is_constant_evaluated()>::value,"");return active()?x+4:x-2;}
constexpr const char* name(){return __func__;}
static_assert(name()[0]=='n',"function name storage in constant activation");
int main(int argc,char**){
 if(active() || __builtin_is_constant_evaluated() || ::__builtin_is_constant_evaluated())return 1;
 if(choose(argc+7)!=6 || work<int>(argc+7)!=6)return 2;
 const bool trial=__builtin_is_constant_evaluated();
 constexpr bool required=__builtin_is_constant_evaluated();
 if(!required)return 3;
 return 0;
}
