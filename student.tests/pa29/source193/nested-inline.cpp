template<class T> struct Box { struct Inner { __attribute__((abi_tag("keep"))) static int value(int x){ return x+7; } }; };
int use(int x){return Box<int>::Inner::value(x);}
int main(){return use(3)!=10;}
