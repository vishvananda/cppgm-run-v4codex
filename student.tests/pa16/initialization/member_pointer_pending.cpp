// Historical implementation-42 reducer: C++11 [expr.const], [expr.unary.op].
// Expected: compile successfully and return 0. Implementation 43 covers this
// explicitly in student.tests/pa16/member_constants.py.
struct X { int a, b; };
constexpr int X::*members[] = {&X::a, &X::b};
int main() { X x = {2, 7}; return x.*members[1] - 7; }
