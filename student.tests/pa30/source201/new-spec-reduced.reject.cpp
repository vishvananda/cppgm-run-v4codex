struct Failure {};
void* operator new(__SIZE_TYPE__);
void* operator new(__SIZE_TYPE__) throw(Failure) { for(;;){} }
