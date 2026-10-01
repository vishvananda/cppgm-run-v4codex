struct DeletedSource { operator int() const; ~DeletedSource()=delete; };
struct PrivateSource { operator int() const; private: ~PrivateSource(); };
static_assert(!__reference_constructs_from_temporary(const int&,DeletedSource),"source temporary is unusable");
static_assert(!__reference_converts_from_temporary(const int&,PrivateSource),"source destructor is inaccessible");
static_assert(__reference_constructs_from_temporary(const int&,DeletedSource&&),"existing xvalue source is not destroyed");
static_assert(__reference_converts_from_temporary(const int&,PrivateSource&),"existing lvalue source is not destroyed");
int dead;
struct Tracked { operator int() const {return 42;} ~Tracked(){++dead;} };
static_assert(__reference_constructs_from_temporary(const int&,Tracked),"converted temporary");
int main(){const int& r=Tracked(); return r==42 && dead==1 ? 0:1;}
