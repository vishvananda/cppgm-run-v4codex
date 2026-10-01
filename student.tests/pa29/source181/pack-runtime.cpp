template<int...N> int pointers(int (*...a)[N]){int result=0;int unused[]={(result+=(*a)[0],0)...};return result;}
template<int...N> int references(int (&...a)[N]){int result=0;int unused[]={(result+=a[0],0)...};return result;}
int main(){int a[1]={7};int b[2]={9,11};return pointers(&a,&b)!=16||references(a,b)!=16;}
