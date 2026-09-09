struct Bits { unsigned value : 2; };
int main() { Bits bits = {1}; unsigned& reference = bits.value; return reference; }
