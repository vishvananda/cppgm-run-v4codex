template<class T> int choose(T) { return 1; }
int choose(int) { return 2; }
int square(int n) { return n*n; }
double square(double n) { return n*n; }
int unary(int n) { return n+1; }
int unary(int a,int b) { return a+b; }
template<class T> T identity(T n) { return n; }
template<class T> T apply(T (*fn)(T),T value) { return fn(value); }
template<class T> T infer(T (*fn)(T)) { return fn(3); }
template<class T> T byref(T (&fn)(T),T value) { return fn(value); }
template<class T> struct Box { T value; };
template<class T> struct View { T value; View(Box<T> b) : value(b.value) {} };
template<class T> int view(const View<T>& v) { return v.value; }
int main() {
    if (choose<int>(0)!=1 || choose(0)!=2) return 1;
    if (apply(square,4)!=16 || byref(square,5)!=25) return 2;
    if (infer(unary)!=4 || apply(identity,6)!=6) return 3;
    { int choose=2; if (!(choose < 3)) return 4; }
    Box<int> box={9};
    if (view<int>(box)!=9) return 5;
    return 0;
}
