struct X{X(int)=delete;};static_assert(!__has_trivial_constructor(X),"");int main(){}
