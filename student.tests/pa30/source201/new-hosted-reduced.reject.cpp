namespace std { struct bad_alloc {}; }
void* operator new(__SIZE_TYPE__);
void* operator new(__SIZE_TYPE__) throw(std::bad_alloc) { for (;;) {} }
