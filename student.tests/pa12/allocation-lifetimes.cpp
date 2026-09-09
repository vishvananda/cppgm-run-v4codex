typedef decltype(sizeof(0)) size_t;
int made, gone, bad, extent_calls;
void *raw;
size_t freed_size;
unsigned count() { ++extent_calls; return 13; }
struct Item {
    int n;
    Item(int seed = 17) : n(seed + made++) {}
    ~Item() { if (n != 17 + made - gone - 1) bad = 1; ++gone; }
    static void *operator new[](size_t n) { return raw = ::operator new[](n); }
    static void operator delete[](void *p, size_t n) noexcept {
        if (p != raw) bad = 1;
        freed_size = n; ::operator delete[](p);
    }
};
int main() {
    Item *items = new Item[count()];
    if (made != 13 || extent_calls != 1 || items[12].n != 29) return 1;
    delete[] items;
    if (gone != 13 || freed_size < 13 * sizeof(Item) || bad) return 2;
    made = gone = 0;
    Item (*rows)[3] = new Item[4][3];
    if (made != 12 || rows[3][2].n != 28) return 3;
    delete[] rows;
    if (gone != 12 || bad) return 4;
    Item *empty = new Item[0];
    delete[] empty;
    return made != 12 || gone != 12 || bad;
}
