int live = 0;
struct Life {
    int value;
    Life(int n) : value(n) { ++live; }
    ~Life() { --live; }
};
int observe(Life const& x = {7}) { return live * 10 + x.value; }
int main() {
    if (observe() + observe() != 44 || live != 0) return 1;
    {
        Life const& local{9};
        if (live != 1 || local.value != 9 || observe() != 27 || live != 1) return 2;
    }
    return live != 0;
}
