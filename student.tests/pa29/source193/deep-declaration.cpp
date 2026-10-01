template<class T> struct Box { struct Inner { struct Leaf { __attribute__((abi_tag("keep"))) static int value(int x); }; }; };
int use(int x){return Box<int>::Inner::Leaf::value(x);}
