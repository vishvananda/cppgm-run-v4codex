template<class T>struct C{int n;static int f(int a,decltype(a) b);int f(long,long);};
template<class U>int C<U>::f(int a,decltype(a) b){return a+b;}
template<class U>int C<U>::f(long a,long b){return n+a+b;}
int add(int a,decltype(a) b){return a+b;}
int main(){C<int> c;c.n=4;return C<int>::f(2,3)!=5 || c.f(2L,3L)!=9 || add(3,4)!=7;}
