template<class T> struct Box { __attribute__((abi_tag("keep"))) static int value(int x); };
template<class T>
int Box<T>::value(int x){return x+7;}
int use(int x){return Box<int>::value(x);}
int main(){return use(3)!=10;}
