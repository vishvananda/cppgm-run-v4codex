struct __attribute__((abi_tag("audit"))) Root154 {
    virtual ~Root154() {}
    virtual int read() const = 0;
};
struct Owner154 : virtual Root154 {
    virtual Root154* self() { return this; }
};
template<int N> struct Value154 : Owner154 {
    int value;
    Value154(int n) : value(n) {}
    Value154* self() { return value ? this : 0; }
    int read() const { return value+N; }
    template<class T> void unused() { T::missing(); }
};
template<class T> __attribute__((pure)) int sample154(const T* p) {
    return p->read();
}
int run154(Owner154* owner) {
    Root154* root = owner->self();
    return root ? sample154(root) : 0;
}
int main(int argc, char**) {
    Value154<7> value(argc);
    Owner154* owner = &value;
    if (run154(owner) != 8 || dynamic_cast<Value154<7>*>(owner) != &value) return 1;
    value.value = 0;
    return run154(owner) != 0;
}
