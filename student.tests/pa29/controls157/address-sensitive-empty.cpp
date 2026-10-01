struct E{}; struct H{[[no_unique_address]] E e;};
struct S {char c; H h; [[no_unique_address]] E e;};
static_assert(sizeof(S)==2,"same empty type at another offset may overlap zero");
static_assert(__builtin_offsetof(S,e)==0,"address zero is free for E");
int main(){}
