// N3485 [temp.arg.nontype], [temp.arg.explicit], [expr.ass], [expr.pseudo].
int calls;
int target(int n) { ++calls; return n + 1; }
struct Proxy {
    int value;
    operator int&() { ++calls; return value; }
};
template<class T, int (*P)(int)>
auto advance(T& t) -> decltype(t += P(1)) { return t += P(1); }
template<class T, int N = 5> int length(T (&)[N]) { return N; }
using I = int;
struct Pointer { int* p; int* operator->() { ++calls; return p; } };
template<class T> void destroy(T& p) { p->~I(); }
template<class T> void dormant() { T::missing(); }
int main() {
    Proxy p{3};
    int& result = advance<Proxy, &target>(p);
    int a[2]{};
    Pointer ptr{&p.value};
    destroy(ptr);
    return calls != 3 || p.value != 5 || &result != &p.value || length<int>(a) != 2;
}
