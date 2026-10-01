constexpr bool active(){return __builtin_is_constant_evaluated();}
template<bool B> struct flag {static constexpr bool value=B;};
static_assert(active(),"");
static_assert(flag<active()>::value,"");
static_assert(sizeof(char[active()?3:5])==3,"");
constexpr int branch(){return active()?12:34;}
int main(){
 constexpr int k=branch(); int x=branch();
 bool a[]={active()}; constexpr bool b[]={active()}; bool c[]={active()};
 if(k!=12||x!=34||a[0]||!b[0]||c[0])return 1;
 const int value=branch(); const int* p=&value;
 return *p==12?0:2;
}
