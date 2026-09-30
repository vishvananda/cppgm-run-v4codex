struct Value { int n; explicit Value(int x) noexcept : n(x) {} };
template<class T> struct Box {
    Box(int x) noexcept(noexcept(T(x))) : value(x) {}
    T value;
};
// No source noexcept query: semantic emission demand must complete the
// constructor boundary, including its unevaluated temporary and parameter.
int main() { Box<Value> b(7); return b.value.n != 7; }
