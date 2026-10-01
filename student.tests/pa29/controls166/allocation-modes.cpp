unsigned long allocated;
alignas(16) unsigned char buffer[1024];
void* operator new[](unsigned long size) { allocated = size; return buffer; }
void operator delete[](void*) noexcept {}
constexpr unsigned bound() { return __builtin_is_constant_evaluated() ? 2 : 5; }
template<int N> constexpr unsigned extent() { return bound() + N; }
template<bool B> constexpr unsigned choice() { return B ? 3 : 7; }
int constructed, destroyed;
struct Item { Item(){++constructed;} ~Item(){++destroyed;} };
int calls;
unsigned dynamic_extent(unsigned n) { ++calls; return n; }
int main(int argc,char**) {
    int* p = new int[bound()];
    if (allocated != 5 * sizeof(int)) return 1;
    delete[] p;
    int (*q)[bound()] = new int[extent<1>()][bound()];
    if (allocated != 6 * 2 * sizeof(int)) return 2;
    delete[] q;
    int* r = new int[choice<__builtin_is_constant_evaluated()>()];
    if (allocated != 3 * sizeof(int)) return 3;
    delete[] r;
    int* s = new int[dynamic_extent(argc+4)];
    if (allocated != 5 * sizeof(int) || calls != 1) return 4;
    delete[] s;
    Item* items = new Item[bound()];
    if (constructed != 5) return 5;
    delete[] items;
    if (destroyed != 5) return 6;
    int* zero = new int[0];
    if (allocated != 0) return 7;
    delete[] zero;
    return 0;
}
