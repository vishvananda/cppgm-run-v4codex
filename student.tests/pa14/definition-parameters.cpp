// N3485 [dcl.fct]/5: callable parameter adjustment does not erase the
// separately retained declaration types used by a function body.
int cv(int*) { return 100; }
int cv(const int*) { return 0; }
int cv(long*) { return 100; }
int cv(const long*) { return 0; }
int twice(int x) { return 2*x; }
long twice(long x) { return 2*x; }
template<class T> struct Parameters {
    T raw(T[3], const T, T(T));
    T (*function())(T);
    T quiet(T) noexcept(sizeof(T)>0);
};
template<class U> U Parameters<U>::raw(U values[3], const U offset, U fn(U)) {
    if (sizeof(values)!=sizeof(U*) || sizeof(fn)!=sizeof(U(*)(U))) return 100;
    return fn(values[0])+offset+cv(&offset);
}
template<class U> U (*Parameters<U>::function())(U) { return twice; }
template<class U> inline U Parameters<U>::quiet(U x) noexcept(sizeof(U)>0) { return x+1; }
int main() {
    Parameters<int> a; Parameters<long> b;
    int values[3]={4,5,6}; long other[3]={7,8,9};
    if (a.raw(values,3,twice)!=11 || b.raw(other,5,twice)!=19) return 1;
    if (a.function()(4)!=8 || b.function()(6)!=12) return 2;
    return a.quiet(2)!=3 || b.quiet(3)!=4;
}
