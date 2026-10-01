#define TAG __attribute__((abi_tag("keep")))
template<class T> struct Box { struct Inner { TAG static int value(int); }; };
template<class T> int Box<T>::Inner::value(int x){return x+7;}
template<> int Box<long>::Inner::value(int x){return x+8;}
template<> struct Box<char> { struct Inner { TAG static int value(int); }; };
int Box<char>::Inner::value(int x){return x+9;}
int main(){return Box<int>::Inner::value(1)!=8 || Box<long>::Inner::value(1)!=9 || Box<char>::Inner::value(1)!=10;}
