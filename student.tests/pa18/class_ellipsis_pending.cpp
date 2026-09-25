// Observed while extending the pack benchmark; unresolved ordinary lowering.
// The selected ellipsis call accepts a trivial class, but emitted LowIR fails
// its variadic-value validator. Kept separate from passing formation controls.
struct Value { int n; };
int consume(...) { return 0; }
int main() { Value v = {1}; return consume(v); }
