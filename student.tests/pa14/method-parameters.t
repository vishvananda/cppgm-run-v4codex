int one(int n){return n+1;}
int two(double n){return n+2;}
template<class T>struct C{int n;int (*f(double))(int);static int (*f(int))(double);};
template<class U>int (*C<U>::f(double))(int){++n;return one;}
template<class U>int (*C<U>::f(int))(double){return two;}
template<class T>struct EnumBox {
 enum E:unsigned char {value=3};
 int n;
 E echo(E x){return x;}
 static int read(E);
 int read(int);
};
template<class U>int EnumBox<U>::read(int k){return n+k;}
int enums(){EnumBox<int> a;EnumBox<long> b;a.n=4;return a.echo(EnumBox<int>::value)!=3 || b.echo(EnumBox<long>::value)!=3 || a.read(2)!=6;}
int main(){C<int> c;c.n=3;return c.f(1.0)(2)!=3 || c.n!=4 || C<int>::f(1)(2.0)!=4 || enums();}
