template<class T> struct Outer { struct Inner {
 __attribute__((abi_tag("z","a","z"))) static int value(int);
}; };
template<class T> __attribute__((abi_tag("z","a"))) int Outer<T>::Inner::value(int x){return x+1;}
int main(){return Outer<int>::Inner::value(4)!=5;}
