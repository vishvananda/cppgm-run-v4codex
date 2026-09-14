// Unfinished PA16 evaluator group: C++11 [expr.const], [expr.unary.op].
// Expected: compile successfully and return 0. Current compiler rejects the
// constexpr array before its uses. This is tracked in pa16/plan.md, not waived.
struct X { int a, b; };
constexpr int X::*members[] = {&X::a, &X::b};
int main() { X x = {2, 7}; return x.*members[1] - 7; }
