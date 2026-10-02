// PA34 cleanup.cpp reducer: [expr.type.conv] versus [expr.cast]/[dcl.name].
typedef unsigned long word;
word shift(unsigned* p) { return (word(*p) << 32); }
int identity(int x) { return x; }
int main() {
    unsigned value = 3;
    if (shift(&value) != (3UL << 32)) return 1;
    if (sizeof(word(*(&value))) != sizeof(word)) return 2;
    int (*fn)(int) = (int (*)(int)) &identity;
    int array[3] = {4, 5, 6};
    int (&ref)[3] = (int (&)[3]) array;
    return fn(ref[2]) != 6;
}
