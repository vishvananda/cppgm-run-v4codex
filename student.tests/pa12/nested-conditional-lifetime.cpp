// [class.temporary]/3: materialized branch results live until the full expression ends.
int alive, created, destroyed;
struct Value {
    int value;
    Value(int n):value(n) { ++alive; ++created; }
    Value(const Value& x):value(x.value) { ++alive; ++created; }
    ~Value() { --alive; ++destroyed; }
};
Value make(int n) { return Value(n); }
int main(int argc,char**) {
    {
        Value x = argc == 2 ? make(2) : argc == 3 ? make(3) : argc == 4 ? make(4) : make(1);
        if (x.value != argc || alive != 1) return 1;
        Value y = argc == 2 ? (argc == 3 ? make(9) : make(2)) : make(argc);
        if (y.value != argc || alive != 2) return 2;
    }
    return alive || created != destroyed;
}
