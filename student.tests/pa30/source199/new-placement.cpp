void* operator new[](unsigned long,void* p) noexcept { return p; }
template<class T> T* at(void* p,unsigned n) { return new (p) T[n]; }
int main(int argc,char**) { int storage[8]; int* p=at<int>(storage,argc+2); p[argc]=31; return storage[argc]-31; }
