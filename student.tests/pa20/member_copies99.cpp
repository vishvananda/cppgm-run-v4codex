int copies, moves;
struct Value {
    Value* self;
    int n;
    Value(int n) noexcept : self(this), n(n) {}
    Value(const Value& v) noexcept : self(this), n(v.n) { ++copies; }
    Value(Value&& v) noexcept : self(this), n(v.n) { ++moves; }
};
struct Aggregate { int prefix; Value first; Value second; };
Aggregate make(Value& first, Value& second) { return {1, first, second}; }
int main() {
    Value a(3), b(4);
    Aggregate result = make(a,b);
    return copies != 2 || moves != 0 || result.prefix != 1 ||
        result.first.n != 3 || result.second.n != 4 ||
        result.first.self != &result.first || result.second.self != &result.second;
}
