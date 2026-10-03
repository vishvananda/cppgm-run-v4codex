// Base ABI entries used only by an unused inline body are not emission roots.
int touched;
struct Base {
    Base() { ++touched; }
    ~Base() { --touched; }
};
struct Unused : Base {
    Unused() {}
    ~Unused() {}
};
inline void unused() { Unused value; }
int main() { return touched; }
