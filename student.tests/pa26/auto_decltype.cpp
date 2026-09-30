template<class F> int apply(F function){
    auto value=function();
    static_assert(__is_same(decltype(value),int),"deduced local type");
    static_assert(__is_same(decltype((value)),int&),"deduced local category");
    auto&& ref=function();
    static_assert(__is_same(decltype(ref),int&&),"deduced reference");
    return value+ref;
}
int seven(){return 7;}
int main(){return apply(seven)!=14;}
