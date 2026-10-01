#if !__has_builtin(__has_trivial_constructor) || !__has_builtin(__has_nothrow_copy) || !__has_builtin(__has_trivial_assign) || !__has_builtin(__has_nothrow_assign) || !__has_builtin(__reference_binds_to_temporary) || !__has_builtin(__reference_constructs_from_temporary) || !__has_builtin(__reference_converts_from_temporary)
#error shared trait registry missing
#endif
template<bool B> struct Choice { static int call(int x){return x+7;} };
template<> struct Choice<true> { static int call(int x){return x+3;} };
template<class T> int use(int x){return Choice<__has_trivial_constructor(T)>::call(x);}
template<class T,class U> struct Lifetime {
 static const bool direct=__reference_constructs_from_temporary(T,U);
 static const bool copy=__reference_converts_from_temporary(T,U);
 static const bool legacy=__reference_binds_to_temporary(T,U);
};
template<class T> struct Holder { T value; };
template<class T> struct Converting { operator T() const; };
struct Noisy { Noisy(){} };
static_assert(__has_trivial_constructor(Holder<int>),"template class completion");
static_assert(!__has_trivial_constructor(Holder<Noisy>),"distinct cached class");
static_assert(Lifetime<const int&,Converting<int>>::direct,"selected conversion");
static_assert(Lifetime<const int&,int>::direct && Lifetime<const int&,int>::legacy,"query operation key");
static_assert(!Lifetime<const int&,int&>::direct,"query operand key");
template<class T> struct Deferred {
 Deferred() noexcept {}
 Deferred(const Deferred&) noexcept {}
 void body(){typename T::invalid value;}
};
static_assert(__has_nothrow_copy(Deferred<int>),"no unrelated body demand");
static_assert(!__has_trivial_copy(Deferred<int>),"user-provided");
int main(int argc,char**) {return use<Holder<int>>(argc)+use<Holder<Noisy>>(argc)-2*argc-10;}
