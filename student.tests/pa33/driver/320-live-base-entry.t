// Live base/complete entries must survive removal of unconditional ABI roots.
int constructed;
int destroyed;
struct Root {
    int value;
    __attribute__((noinline)) Root() : value(7) { ++constructed; }
    __attribute__((noinline)) ~Root() { ++destroyed; }
};
struct Base : virtual Root {
    __attribute__((noinline)) Base() { value += 3; }
    __attribute__((noinline)) ~Base() { value -= 3; }
};
struct Derived : Base {
    __attribute__((noinline)) Derived() { value += 5; }
    __attribute__((noinline)) ~Derived() { value -= 5; }
};
int main() {
    {
        Derived derived;
        if (derived.value != 15 || constructed != 1 || destroyed != 0) return 1;
        Base base;
        if (base.value != 10 || constructed != 2 || destroyed != 0) return 2;
    }
    return destroyed == 2 ? 0 : 3;
}
