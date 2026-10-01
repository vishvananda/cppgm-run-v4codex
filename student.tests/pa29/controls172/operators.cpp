template<class...T> int mul(T...x){return (...*x);}
template<class...T> int div(T...x){return (.../x);}
template<class...T> int mod(T...x){return (...%x);}
template<class...T> int bits(T...x){return (...^x);}
template<class...T> int shifts(T...x){return (...<<x);}
template<class...T> bool cmp(T...x){return (...<x);}
template<class...T> int* ptr(int*p,T...x){return (p+...+x);}
struct A{int n;};
template<class...T> int& member(A&a,T...p){return (a.*... .*p);}
int main(){int a[10];A v={7};member(v,&A::n)=9;return mul(2,3,4)!=24 || div(24,3,2)!=4 || mod(25,12,3)!=1 || bits(1,3,7)!=5 || shifts(1,2,3)!=32 || !cmp(1,2,3) || ptr(a,2,3)!=a+5 || v.n!=9;}
