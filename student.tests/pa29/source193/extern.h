#define TAG __attribute__((abi_tag("keep")))
template<class T> struct Box { struct Inner { TAG static int value(int); }; };
template<class T> int Box<T>::Inner::value(int x){return x+7;}
