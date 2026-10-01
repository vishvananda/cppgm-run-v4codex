struct Pair { int value; };
constexpr int mutate() {
    Pair p{1};
    int& r = p.value;
    r = 2;
    return p.value;
}
static_assert(mutate() == 2, "C++14 aggregate-subobject mutation");
int main() { return mutate() == 2 ? 0 : 1; }
