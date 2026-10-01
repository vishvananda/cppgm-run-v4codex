#define TAG __attribute__((abi_tag("keep")))
template<class T> struct Box {
 struct Inner { TAG static int value(int); };
};
Box<int>::Inner early_object;
int (*early_address)(int)=&Box<int>::Inner::value;
int early_call(int x){return Box<int>::Inner::value(x);}
template<class T> int Box<T>::Inner::value(int x){return x+7;}
int (*late_address)(int)=&Box<int>::Inner::value;
int main(){return early_address!=late_address || early_call(2)!=9 || late_address(3)!=10;}
