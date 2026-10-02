#include <new>
#include <cstdlib>
int allocations, releases;
void* operator new[](std::size_t n) { ++allocations; return std::malloc(n); }
void operator delete[](void* p) noexcept { ++releases; std::free(p); }
