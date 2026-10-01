template<class T> struct box {};
box(int x, int y=sizeof(x))->box<int>;
template<class T> box(T x, int y=sizeof(x), bool z=noexcept(x+x))->box<T>;
template<class T> int f(T x,int y=sizeof(x)){return y;}
template<class T> int array(T x[3],int n=sizeof(x)){return n;}
template<class T> int refs(T& x,int n=sizeof(x)){return n;}
template<class... T> int count(int n=sizeof...(T)){return n;}
int main(){int a[3];return f(7)!=sizeof(int)||array(a)!=sizeof(int*)||refs(a)!=sizeof(a)||count<int,char>()!=2;}
