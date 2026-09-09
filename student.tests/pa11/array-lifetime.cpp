int constructed, destroyed, checksum;
struct Element {
    int value;
    Element() : value(++constructed) {}
    ~Element() { ++destroyed; checksum = checksum + value; }
};
struct Holder { Element elements[3]; };
int count() {
    Element earlier;
    Element items[12];
    return items[11].value + constructed - destroyed;
}
int main() {
    { Element matrix[2][2]; }
    if (constructed != 4 || destroyed != 4 || checksum != 10) return 1;
    { Holder holder; }
    if (constructed != 7 || destroyed != 7 || checksum != 28) return 2;
    if (count() != 33 || constructed != 20 || destroyed != 20 || checksum != 210) return 3;
    typedef int Int;
    Int x = 0;
    (&x)->~Int();
    return x;
}
