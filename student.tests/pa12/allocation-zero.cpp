unsigned calls;
unsigned extent() { ++calls; return 19; }
struct Value { long n; int m; };
int main() {
    int *scalar = new int();
    if (*scalar) return 1;
    delete scalar;
    int *paren = new (int)(int(7));
    if (*paren != 7) return 4;
    delete paren;
    bool *bits = new bool[extent()]();
    for (unsigned i = 0; i != 19; ++i) if (bits[i]) return 2;
    delete[] bits;
    Value *values = new Value[extent()]();
    for (unsigned i = 0; i != 19; ++i) if (values[i].n || values[i].m) return 3;
    delete[] values;
    return calls != 2;
}
