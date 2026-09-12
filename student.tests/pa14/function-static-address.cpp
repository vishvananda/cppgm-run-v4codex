struct Factory {
    static int value() { return 7; }
};
template<class Tag> int call(int fn()) { return fn(); }
int through_reference(int (&fn)()) { return call<void>(fn); }
int main() {
    Factory object;
    int (*p)() = object.value;
    if (p() != 7 || call<int>(object.value) != 7) return 1;
    if (through_reference(*p) != 7) return 2;
    return call<long>(Factory::value) == 7 ? 0 : 3;
}
