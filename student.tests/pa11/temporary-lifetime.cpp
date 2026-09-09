int trace;
struct Guard {
    int number;
    Guard(int n) : number(n) { trace = trace * 10 + n; }
    ~Guard() { trace = trace * 10 + number; }
    int value() const { return number; }
};
int sum() { return Guard(1).value() + Guard(2).value(); }
struct NoInline {
    int x;
    __attribute__((noinline)) NoInline() : x(7) {}
    int value() const { return x; }
};
int main() {
    int result = sum();
    if (result != 3 || trace != 1221) return 1;
    trace = 0;
    Guard(3).value();
    if (trace != 33) return 2;
    return NoInline().value() - 7;
}
