struct Bits { unsigned value : 2; };
int main() { Bits bits; unsigned* pointer = &bits.value; return 0; }
