struct X{template<int...N> int run(int (*...a)[N]) const;};
template<int...N> int X::run(int (*...a)[N]) const{int total=0;int used[]={(total+=(*a)[0],0)...};return total;}
int main(){int a[1]={4};int b[3]={8,1,2};return X().run(&a,&b)!=12;}
