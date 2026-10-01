_Atomic(int) value=3;
int conversions=0;
struct Ref { operator _Atomic(int)&() { ++conversions;return value; } };
static_assert(__reference_constructs_from_temporary(const int&,Ref),"conversion snapshot");
static_assert(__reference_converts_from_temporary(const int&,Ref),"copy conversion snapshot");
static_assert(!__reference_constructs_from_temporary(_Atomic(int)&,Ref),"atomic identity");
struct Reader { int read(const int& x) { value=19;return x; } };
struct Pointer { Reader* p;Reader& operator*() const {return *p;} };
template<class T> int invoke(T p) { return __builtin_invoke(&Reader::read,p,value); }
template<class T> int snapshot() { const int& r=value;value=11;return r; }
int main(){
 const int& r=Ref{};value=7;if(r!=3 || conversions!=1)return 1;
 Reader reader;Pointer p={&reader};
 if(invoke(p)!=7 || value!=19)return 2;
 if(snapshot<int>()!=19 || value!=11)return 3;
 _Atomic(int)& a=Ref{};a=23;
 return value==23 && conversions==2?0:4;
}
