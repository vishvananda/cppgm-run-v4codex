template<class T> struct Box { __attribute__((abi_tag("keep"))) static int value(int x); };
int use(int x){return Box<int>::value(x);}
