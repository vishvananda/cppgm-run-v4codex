template<class T> struct Box { struct Inner { struct Leaf { __attribute__((abi_tag("keep"))) static int value(int x); }; }; };
template<class T>
__attribute__((abi_tag("keep"))) int Box<T>::Inner::Leaf::value(int x){return x+7;}
int use(int x){return Box<int>::Inner::Leaf::value(x);}
int main(){return use(3)!=10;}
