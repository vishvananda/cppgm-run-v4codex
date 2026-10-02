template<int W> __attribute__((noinline)) unsigned char* fill(unsigned char* first, long count, const unsigned char* value) {
    unsigned char* last = first + count * W;
    unsigned char* p = first;
    for (; p != last; ++p) *p = *value;
    return p;
}
int main(int argc, char**) {
    unsigned char data[64] = {};
    data[5] = argc + 17;
    unsigned char* end = fill<1>(data, 64, data + 5);
    if (end != data + 64) return 1;
    for (int i = 0; i < 64; ++i) if (data[i] != argc + 17) return 2;
    return fill<2>(0, 0, 0) != 0;
}
