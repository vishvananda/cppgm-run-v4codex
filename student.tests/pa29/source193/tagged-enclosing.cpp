template<class T> struct __attribute__((abi_tag("outer"))) Box {
 struct __attribute__((abi_tag("inner"))) Inner {
  __attribute__((abi_tag("function"))) static int value(int);
 };
};
template<class T> int Box<T>::Inner::value(int x){return x+1;}
int main(){return Box<int>::Inner::value(4)!=5;}
