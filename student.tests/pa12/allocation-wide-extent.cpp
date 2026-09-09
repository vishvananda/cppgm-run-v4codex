typedef decltype(sizeof(0)) size_t;
struct Tag {};
size_t requested;
void *operator new[](size_t n, Tag) noexcept(true) { requested = n; return 0; }
unsigned extent() { volatile unsigned n = 1073741824u; return n; }
int main() {
    unsigned long *p = new (Tag()) unsigned long[extent()]();
    return p != 0 || requested != 8589934592ul;
}
