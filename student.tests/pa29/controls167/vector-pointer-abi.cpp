typedef int I4 __attribute__((vector_size(16)));
typedef int I8 __attribute__((vector_size(32)));
typedef float F4 __attribute__((vector_size(16)));
static_assert(__is_literal_type(I4) && __is_trivially_copyable(I4),"vector properties");
int extent(const I4*){return 16;}
int extent(const I8*){return 32;}
int extent(const F4*){return 64;}
int main(){return extent((I4*)0)+extent((I8*)0)+extent((F4*)0)==112?0:1;}
