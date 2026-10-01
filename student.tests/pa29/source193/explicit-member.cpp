#define TAG __attribute__((abi_tag("keep")))
template<class T> struct Box { TAG static int value(int); template<class U> TAG static int member(U); };
template<class T> int Box<T>::value(int x){return x+7;}
template<class T> template<class U> int Box<T>::member(U x){return x+8;}
template<> int Box<long>::value(int x){return x+9;}
template<> template<> int Box<long>::member<int>(int x){return x+10;}
int main(){return Box<int>::value(1)!=8 || Box<int>::member(1)!=9 || Box<long>::value(1)!=10 || Box<long>::member(1)!=11;}
