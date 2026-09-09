struct Bits {
    unsigned first : 1;
    unsigned second : 2;
    int signed_value : 3;
    unsigned char guard;
    Bits() : first(1), second(2), signed_value(-1), guard(9) {}
};
int calls;
Bits& touch(Bits& b) { ++calls; return b; }
struct Narrow { unsigned char a : 3; unsigned char b : 3; unsigned char guard; };
struct Mixed { unsigned char prefix; unsigned field : 3; };
struct Wide { unsigned char bits : 12; unsigned char tail; };
struct Empty {};
struct Nested : Empty { int value; };
struct Wrapper : Empty { Nested member; };
int main() {
    Bits value;
    if (value.first != 1 || value.second != 2 || value.signed_value != -1 || value.guard != 9) return 1;
    ++touch(value).second;
    if (calls != 1 || value.second != 3) return 2;
    if (value.second++ != 3 || value.second != 0 || value.first != 1 || value.guard != 9) return 3;
    value.signed_value = 7;
    if (value.signed_value != -1) return 4;
    Narrow narrow = {1,2,9};
    narrow.b = 15;
    if (narrow.a != 1 || narrow.b != 7 || narrow.guard != 9 || sizeof(Narrow) != 2) return 5;
    Wide wide;
    wide.bits = 255; wide.tail = 11;
    if (wide.bits != 255 || wide.tail != 11 || sizeof(Wide) != 3) return 6;
    // [conv.prom]/5 uses the field's value range, not its declared rank.
    value.first = 0;
    if ((value.first - 1) / 2 != 0) return 7;
    const unsigned& snapshot = value.second;
    value.second = 2;
    if (snapshot != 0) return 8;
    Mixed mixed = {19,5};
    if (mixed.prefix != 19 || mixed.field != 5) return 9;
    return sizeof(Wrapper) != 8;
}
