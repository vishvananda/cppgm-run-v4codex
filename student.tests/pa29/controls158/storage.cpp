using ByteInt __attribute__((aligned(1))) = int;
typedef int Word __attribute__((aligned(8)));
template<int N> using Aligned __attribute__((aligned(N))) = int;
template<int N> struct Holder {
 using Row __attribute__((aligned(N))) = int[4];
 char c; Row row;
};
ByteInt a[4]; ByteInt* p=a;
static_assert(__is_same(ByteInt,int),"canonical identity");
static_assert(__alignof__(ByteInt)==1,"alias alignment");
static_assert(__alignof__(Aligned<1>)==1,"alias specialization");
static_assert(__alignof__(Aligned<8>)==8,"distinct alignment arguments");
static_assert(__alignof__(a)==1,"array alignment");
static_assert(__alignof__(a[1])==1,"array element");
static_assert(__alignof__(*p)==1,"pointer pointee");
static_assert(__alignof__(*(&a[1]))==1,"address and indirection");
static_assert(__alignof__((p+2)[0])==1,"pointer arithmetic");
static_assert(__alignof__(Holder<16>)==16,"dependent member alias");
static_assert(__builtin_offsetof(Holder<16>,row)==16,"member layout");
template<class T> unsigned long alignment(){ return __alignof__(*p)+__alignof__(T); }
int main(){ Holder<16> h={}; h.c=7; h.row[1]=13; a[1]=9;
 return h.c!=7 || h.row[1]!=13 || *(&a[1])!=9 || alignment<char>()!=2; }
