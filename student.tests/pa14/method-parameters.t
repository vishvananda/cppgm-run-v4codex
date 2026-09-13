int one(int n){return n+1;}
int two(double n){return n+2;}
template<class T>struct C{int n;int (*f(double))(int);static int (*f(int))(double);};
template<class U>int (*C<U>::f(double))(int){++n;return one;}
template<class U>int (*C<U>::f(int))(double){return two;}
int main(){C<int> c;c.n=3;return c.f(1.0)(2)!=3 || c.n!=4 || C<int>::f(1)(2.0)!=4;}
