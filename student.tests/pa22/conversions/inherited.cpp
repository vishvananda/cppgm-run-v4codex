struct Pad { int padding; };
struct Bool { int value; explicit operator bool() const { return value != 0; } };
struct Int { int value; operator int() const { return value; } };
struct D : Pad, Bool, Int {};
struct Override : D { explicit operator bool() const { return false; } };
template<class T> bool truth(const T& t) { return bool(t); }
int main() {
    D d; d.Bool::value = 9; d.Int::value = 13;
    if (!truth(d) || int(d) != 13) return 1;
    d.Bool::value = 0;
    if (truth(d)) return 2;
    Override o; o.Bool::value = 4; o.Int::value = 7;
    if (truth(o) || int(o) != 7) return 3;
    return 0;
}
