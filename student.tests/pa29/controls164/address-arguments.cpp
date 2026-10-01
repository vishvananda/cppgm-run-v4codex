bool has(const char* s,const char* part){for(;*s;++s){const char* a=s;const char* b=part;while(*a && *b && *a==*b){++a;++b;}if(!*b)return true;}return false;}
int global;
int fun(int x){return x;}
struct C{int field; int method(){return field;}};
template<int* P> const char* ptr(){return __PRETTY_FUNCTION__;}
template<int& R> const char* ref(){return __PRETTY_FUNCTION__;}
template<int(*F)(int)> const char* function(){return __PRETTY_FUNCTION__;}
template<int C::* P> const char* data(){return __PRETTY_FUNCTION__;}
template<int(C::*F)()> const char* member(){return __PRETTY_FUNCTION__;}
int main(){
 if(!has(ptr<&global>(),"P = &global") || !has(ptr<nullptr>(),"P = nullptr"))return 1;
 if(!has(ref<global>(),"R = global") || !has(function<&fun>(),"F = &fun"))return 2;
 if(!has(data<&C::field>(),"P = &C::field") || !has(data<nullptr>(),"P = nullptr"))return 3;
 if(!has(member<&C::method>(),"F = &C::method"))return 4;
 return 0;
}
