template<class T> struct Box { template<class U> struct Inner { __attribute__((abi_tag("keep"))) static int value(int x); }; };
template<class T> template<class U>
__attribute__((abi_tag("keep"))) int Box<T>::Inner<U>::value(int x){return x+7;}
int use(int x){return Box<int>::Inner<long>::value(x);}
int main(){return use(3)!=10;}
