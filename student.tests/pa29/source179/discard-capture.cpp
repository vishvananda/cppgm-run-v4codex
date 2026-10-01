struct Pair{int a;int& b;};
template<class T>int f(){if constexpr(true){int x=3;auto [a,b]=Pair{2,x};auto g=[&](){b+=a;return b;};return g()+x;}else{auto [x,y]=T::missing;return x+y;}}
int main(){return f<int>()!=10;}
