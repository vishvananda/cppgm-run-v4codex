typedef int ByteInt __attribute__((aligned(1)));
ByteInt* pass(ByteInt* p){static_assert(__alignof__(*p)==1,"parameter");return p;}
struct S{ByteInt member; ByteInt* pointer;};
S s;
static_assert(__alignof__(s.member)==1,"member");
static_assert(__alignof__(*s.pointer)==1,"member pointer");
static_assert(__alignof__(*pass(s.pointer))==1,"return type");
static_assert(__alignof__(*static_cast<ByteInt*>(s.pointer))==1,"cast");
template<class T> ByteInt* templated(ByteInt* p){return p;}
static_assert(__alignof__(*templated<int>(s.pointer))==1,"template return");
ByteInt* (*callback)(ByteInt*)=pass;
static_assert(__alignof__(*callback(s.pointer))==1,"indirect return");
int main(){ByteInt a=7;return *pass(&a)!=7;}
