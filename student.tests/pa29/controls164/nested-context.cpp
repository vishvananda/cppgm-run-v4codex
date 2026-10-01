bool has(const char* s,const char* part){for(;*s;++s){const char* a=s;const char* b=part;while(*a && *b && *a==*b){++a;++b;}if(!*b)return true;}return false;}
namespace scope {inline namespace first {inline namespace second {struct Tag {};}}}
template<class T> struct A {template<class U> struct B {template<int N> static const char* text(){return __PRETTY_FUNCTION__;}};};
template<class T> const char* text(){return __PRETTY_FUNCTION__;}
template<class T> using Alias=T;
struct Conversion {operator int()const{return has(__func__,"operator int")?3:0;}};
int main(){
 const char* s=A<int>::B<double>::text<9>();
 if(!has(s,"T = int") || !has(s,"U = double") || !has(s,"N = 9"))return 1;
 if(!has(text<scope::Tag>(),"T = scope::Tag"))return 2;
 if(!has(text<volatile Alias<const int> >(),"const volatile"))return 3;
 if(int(Conversion())!=3)return 4;
 return 0;
}
