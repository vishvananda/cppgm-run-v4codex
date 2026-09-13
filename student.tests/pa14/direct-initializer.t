// N3485 8.2 [dcl.ambig.res]/1, 8 [dcl.decl], 8.3.5 [dcl.fct].
// Literal operands cannot be parameter declarators; possible function
// declarations still win when the parentheses contain a declarator or type.
template<class T> int direct() {
    struct Cell { T value; Cell(T x) : value(x) {} };
    Cell first(T(5));
    Cell second(T(-3));
    Cell function(T(parameter));
    Cell empty(T());
    return first.value + second.value;
}
int main() { return direct<int>() != 2 || direct<long>() != 2; }
