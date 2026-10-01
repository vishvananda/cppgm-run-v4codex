template<class T> struct Box { template<class U> struct Inner { __attribute__((abi_tag("keep"))) static int value(int x); }; };
int use(int x){return Box<int>::Inner<long>::value(x);}
