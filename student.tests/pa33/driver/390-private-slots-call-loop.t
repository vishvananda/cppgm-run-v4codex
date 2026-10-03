volatile int observed;
__attribute__((noinline)) int observe(int value) { ++observed; return value * 3; }
__attribute__((noinline)) void change(int* value) { *value += 2; }
__attribute__((noinline)) void check(int value) { if (value == 7) throw value; }
int main() {
    int total = 0, escaped = 0, caught = 0;
    for (int i = 0; i < 10; ++i) {
        total += observe(i);
        change(&escaped);
        try { check(i); } catch (int value) { caught += value; }
    }
    return total != 135 || escaped != 20 || caught != 7 || observed != 10;
}
