// N3485 [class.conv.fct]: conversion functions may be virtual.
struct B {
    virtual operator int() const { return 1; }
    virtual operator long() const { return 2; }
};
struct D : B {
    operator int() const override { return 7; }
    operator long() const override { return 8; }
};
int main() {
    D object;
    B& base = object;
    int implicit = base;
    int explicit_cast = static_cast<int>(base);
    long other_slot = base;
    return implicit != 7 || explicit_cast != 7 || other_slot != 8 || base.B::operator int() != 1;
}
