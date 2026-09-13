// N3485 [dcl.fct], [temp.deduct], [dcl.ref], [dcl.type.simple].
// Body parameter types preserve cv and reference collapsing independently of
// the canonical callable signature's array/function adjustment.
int qualification(int*) { return 1; }
int qualification(const int*) { return 2; }
template<class T> int top_cv(const T value) { return qualification(&value); }
template<class T> T add(T a,decltype(a) b) { return a+b; }
template<class U> U renamed(const U value);
template<class V> V renamed(const V value) { return value; }
template<class T> T (&array_ref(T (&values)[3]))[3] { return values; }
template<class T> int array_parameter(const T values[3]) { return values[2]; }
int increment(int value) { return value+1; }
long increment_long(long value) { return value+2; }
template<class T> T (*factory(T (*fn)(T)))(T) { return fn; }
template<class F> int function_parameter(F fn) { return fn(8); }
template<class T> int empty(void) { return sizeof(T); }
struct Short { typedef short value_type; };
struct Long { typedef long value_type; };
template<class T> typename T::value_type read(const typename T::value_type& v) { return v; }
int main() {
    int n=7;
    if (top_cv(n)!=2 || top_cv<int&>(n)!=1) return 1;
    if (add(2,3)!=5 || add(4L,7L)!=11 || renamed(n)!=7) return 2;
    int values[3]={3,5,9};
    array_ref(values)[1]=6;
    if (&array_ref(values)!=&values || values[1]!=6 || array_parameter(values)!=9) return 3;
    if (factory(increment)(2)!=3 || factory(increment_long)(4)!=6) return 4;
    if (function_parameter<int(int)>(increment)!=9 || empty<long>()!=8) return 5;
    short s=4; long l=11;
    return read<Short>(s)!=4 || read<Long>(l)!=11 || sizeof(read<Short>(s))!=sizeof(short);
}
