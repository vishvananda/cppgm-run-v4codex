int ended;
struct Number {
    int value;
    Number(int n, int add = 2) : value(n + add) {}
    ~Number() { ++ended; }
};
int read(const Number& number) { return number.value; }
struct Pair {
    int result;
    Pair(const Number& a, const Number& b) : result(a.value + b.value) {}
};
struct Empty {};
int empty(Empty) { return 3; }
int main() {
    int read_value = read(5);
    if (read_value != 7 || ended != 1) return 1;
    Pair p(1, 4);
    if (p.result != 9 || ended != 3) return 2;
    Empty e;
    return empty(e) == 3 ? 0 : 3;
}
