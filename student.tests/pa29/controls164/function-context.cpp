extern "C" int puts(const char*);
bool has(const char* s,const char* part){for(;*s;++s){const char* a=s;const char* b=part;while(*a && *b && *a==*b){++a;++b;}if(!*b)return true;}return false;}
bool same(const char* a,const char* b){while(*a && *a==*b){++a;++b;}return *a==*b;}
namespace api { inline namespace version { struct Item { int x; }; enum Code { one }; } }
template<class T> const char* text(){return __PRETTY_FUNCTION__;}
template<class T> int array_bound(){static_assert(__PRETTY_FUNCTION__[0]=='i',"return type");return sizeof(__PRETTY_FUNCTION__);}
template<class T, int N> struct Owner {
 template<unsigned Depth=0> static const char* name(){return __PRETTY_FUNCTION__;}
 const char* member() const & {return __PRETTY_FUNCTION__;}
};
template<class... T> const char* pack(){return __PRETTY_FUNCTION__;}
template<int... N> const char* numbers(){return __PRETTY_FUNCTION__;}
template<class T> struct Partial;
template<class T> struct Partial<T*> {static const char* name(){return __PRETTY_FUNCTION__;}};
template<class T> const char* simple(){return __func__;}
template<bool V> const char* boolean(){return __PRETTY_FUNCTION__;}
int main(){
 if(!same(__func__,"main") || !same(__FUNCTION__,"main"))return 1;
 if(!has(__PRETTY_FUNCTION__,"int main()"))return 2;
 if(!has(text<api::Item>(),"T = api::Item") || has(text<api::Item>(),"version"))return 3;
 if(!has(text<api::Code>(),"T = api::Code"))return 4;
 if(!has(text<const int*>(),"T = int const*"))return 5;
 if(!has(text<int(&)[3]>(),"T = int(&)[3]"))return 6;
 if(!has(text<int(*)(double)>(),"T = int(*)(double)"))return 7;
 if(!has(text<int api::Item::*>(),"api::Item::*"))return 8;
 if(!has(Owner<long,7>::name<2>(),"T = long int, N = 7, Depth = 2"))return 9;
 Owner<int,4> o;if(!has(o.member(),"const &") || !has(o.member(),"N = 4"))return 10;
 if(!has(pack<int,long>(),"T = <int, long int>") || !has(pack<>(),"T = <>"))return 11;
 if(!has(numbers<1,-2,3>(),"N = <1, -2, 3>"))return 12;
 if(!has(Partial<int*>::name(),"T = int"))return 13;
 if(!same(simple<int>(),"simple") || !same(simple<long>(),"simple"))return 14;
 if(!has(boolean<true>(),"V = true"))return 15;
 if(array_bound<int>()<=20 || array_bound<long>()<=array_bound<int>())return 16;
 if(text<int>()!=text<int>() || same(text<int>(),text<long>()))return 17;
 return 0;
}
