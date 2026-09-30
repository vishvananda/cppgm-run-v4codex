template<class T> using decayed = __decay(T);
static_assert(__is_same(decayed<const int&>, int), "reference cv");
static_assert(__is_same(decayed<int[3]>, int*), "array");
static_assert(__is_same(decayed<const int(&)[3]>, const int*), "array reference");
static_assert(__is_same(decayed<int(int)>, int(*)(int)), "function");
static_assert(__is_same(decayed<int* const>, int*), "pointer cv");
static_assert(__is_same(decayed<void>, void), "void");
template<class T> __attribute__((noinline)) decayed<T> result(T* p) { return *p; }
int main() { int value = 11; return result(&value) == 11 ? 0 : 1; }
