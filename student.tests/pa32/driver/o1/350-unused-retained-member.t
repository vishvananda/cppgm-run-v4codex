struct Empty { ~Empty() {} };
struct Unused : Empty { ~Unused() {} };
inline void unused() { Unused value; }
inline int unused_local() {
    struct Local { __attribute__((noinline)) int value() { return 23; } };
    Local value;
    return value.value();
}
int main() { return 0; }
