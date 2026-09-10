int live;
int destroyed;
struct Value {
    int member;
    Value(int n) : member(n) { ++live; }
    ~Value() { --live; destroyed += member; }
};
int run(int choose) {
    int existing = 17;
    {
        const int& result = choose == 0 ? Value(3).member :
            choose == 1 ? Value(5).member : static_cast<int&&>(existing);
        if (live != (choose != 2)) return 1;
        if (result != (choose == 0 ? 3 : choose == 1 ? 5 : 17)) return 2;
    }
    return live != 0;
}
int early(int choose) {
    const int& result = choose ? Value(7).member : Value(11).member;
    if (live != 1) return 1;
    if (choose) return result != 7;
    return result != 11;
}
int main() {
    if (run(0) || run(1) || run(2) || early(0) || early(1)) return 1;
    for (int i = 0; i < 3; ++i) {
        const int& result = i ? Value(13).member : Value(17).member;
        if (live != 1) return 2;
        if (i == 1) continue;
        if (i == 2) break;
    }
    return live || destroyed != 69;
}
