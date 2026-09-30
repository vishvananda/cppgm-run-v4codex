// The target's integer atomic family must preserve widths and return conventions.
template<class T> int check() {
    volatile T value = T(4);
    T old = __atomic_fetch_add(&value, T(3), 0);
    T updated = __atomic_add_fetch(&value, T(5), 5);
    return old != T(4) || updated != T(12) || value != T(12);
}
int side_effects;
volatile int counters[2] = {9,20};
volatile int* pointer() { ++side_effects; return &counters[0]; }
int amount() { ++side_effects; return 7; }
int order() { ++side_effects; return 2; }
extern "C" long increment(volatile long* value) { return __atomic_fetch_add(value,1L,5); }
template<class T> T* pointer_add(T* volatile* value, long delta) {
    return __atomic_fetch_add(value,delta,1);
}
static_assert(__is_same(decltype(__atomic_fetch_add((int* volatile*)0,1,0)),int*), "pointer result");
extern "C" int controls() {
    int data[4]; int* volatile ptr=data;
    if (pointer_add(&ptr,sizeof(int))!=data || ptr!=data+1) return 4;
    if (__atomic_add_fetch(&ptr,sizeof(int),4)!=data+2 || ptr!=data+2) return 5;
    int old = __atomic_fetch_add(pointer(),amount(),order());
    if (old!=9 || counters[0]!=16 || counters[1]!=20 || side_effects!=3) return 1;
    if (check<char>() || check<signed char>() || check<unsigned char>() || check<short>() ||
        check<unsigned short>() || check<int>() || check<unsigned>() || check<long>() ||
        check<unsigned long>() || check<long long>() || check<unsigned long long>() ||
        check<wchar_t>() || check<char16_t>() || check<char32_t>() || check<__int128>() ||
        check<unsigned __int128>()) return 2;
    volatile signed char wrap=127;
    if (__atomic_fetch_add(&wrap,1,5)!=127 || wrap!=-128) return 3;
    return 0;
}
