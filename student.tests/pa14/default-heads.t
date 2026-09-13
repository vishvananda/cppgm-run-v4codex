// N3485 [dcl.fct.default]/5, [temp.inst]/2: the default owns a separate
// definition, including the original declaration's template parameters.
template<class T> int choose(int n=sizeof(T));
template<class U> int choose(int n) { return n; }
template<class T> int late(int n=sizeof(T));
int before_definition() { return late<int>(); }
template<class U> int late(int n) { return U(n)+sizeof(U); }
namespace Named {
    const int offset=3;
    template<class T> int qualified(int n=sizeof(T)+offset);
}
template<class U> int Named::qualified(int n) { return n; }
int main() {
    return choose<int>()!=sizeof(int) || choose<long>()!=sizeof(long) ||
        before_definition()!=2*sizeof(int) || late<long>()!=2*sizeof(long) ||
        Named::qualified<int>()!=sizeof(int)+3;
}
