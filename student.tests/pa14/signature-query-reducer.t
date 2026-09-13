// N3485 [dcl.fct]/5: function parameters adjust to function pointers.
int twice(int n) { return 2*n; }
template<class T> struct C {
    T apply(T values[3], const T offset, T fn(T)) { return fn(values[0])+offset; }
};
int main() { C<int> c; int values[3]={4,5,6}; return c.apply(values,3,twice)!=11; }
