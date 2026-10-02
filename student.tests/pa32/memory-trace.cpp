struct Pair { long x, y; };
template<class T> __attribute__((noinline)) long twice(T* object) {
    long first = object->x;
    long second = object->x;
    return first + second;
}
__attribute__((noinline)) long choose(Pair* object, bool right) {
    long x = object->x, y = object->y;
    return right ? object->y : object->x;
}
long observed;
__attribute__((noinline)) long capture(int throws) {
    long before = observed;
    try {
        observed = 17;
        if (throws) throw 3;
        observed = 19;
    } catch (int) { return before + observed; }
    return before + observed;
}
int main(int argc, char**) {
    Pair pair = {argc + 7, argc + 11};
    if (twice(&pair) != 2 * (argc + 7)) return 1;
    if (choose(&pair, false) != argc + 7 || choose(&pair, true) != argc + 11) return 2;
    observed = 3;
    if (capture(1) != 20) return 3;
    observed = 5;
    return capture(0) != 24;
}
