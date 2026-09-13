// N3485 [basic.scope.class]/1, [class.mem]/2: a non-static data-member
// initializer sees declarations that follow it in the enclosing class.
template<class T> struct C {
    int value=make();
private:
    static int make() { return 7; }
};
int main() { C<int> c; return c.value!=7; }
