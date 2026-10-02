// Reduced from the empty-array nested type in the selected hosted <array>.
#if !__has_builtin(__builtin_trap)
#error implemented builtin must be advertised
#endif
static_assert(noexcept(__builtin_trap()), "trap cannot unwind");
template<class T> struct box {
    struct empty {
        T& operator[](unsigned long) const noexcept { __builtin_trap(); }
    };
};
int main(int argc, char**) {
    if (argc == 2) { box<int>::empty value; return value[0]; }
    if (argc == 3) ::__builtin_trap();
    return 0;
}
