struct Z {int a[0];};
template<class T> auto size(T& x)->decltype(sizeof(x)){return sizeof(x);}
template<class T> auto pointer(T* p)->decltype(p+1){return p+1;}
template<class T> auto make()->decltype(new T()){return new T();}
int main(){Z z{};auto p=make<Z>();delete p;return size(z)!=0||pointer(&z)!=&z;}
