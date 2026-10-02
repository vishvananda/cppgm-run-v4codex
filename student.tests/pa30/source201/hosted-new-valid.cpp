#include <new>
void* operator new(__SIZE_TYPE__) { throw std::bad_alloc(); }
int main(){try{::operator new(4);}catch(const std::bad_alloc&){return 0;}return 1;}
