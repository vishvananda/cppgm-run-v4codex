// N3485 [basic.scope.class]/1, [class.mem]/2: default arguments are
// complete-class contexts even when the selected member is declared later.
template<class T> struct C {
    T get(T value=make()) { return value; }
private:
    static T make() { return T(7); }
};
int main() { C<int> c; return c.get()!=7; }
