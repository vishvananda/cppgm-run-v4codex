struct Bits { unsigned value : 1; };
int main() { Bits bits = {0}; return (bits.value - 1) / 2; }
