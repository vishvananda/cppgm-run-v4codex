#define TAG(x) __attribute__((abi_tag(x)))
template<class T> struct Box {
 struct Inner {
  TAG("drop") static int value(int);
  TAG("keep") static int value(long);
  TAG("inline") static int inside(int x){return x+3;}
  TAG("data") static int data;
  TAG("method") int method(int) const;
  template<class U> TAG("member") static int member(U);
 };
};
template<class T> int Box<T>::Inner::value(int x){return x+1;}
template<class T> TAG("keep") int Box<T>::Inner::value(long x){return x+2;}
template<class T> int Box<T>::Inner::method(int x) const {return x+data;}
template<class T> template<class U> int Box<T>::Inner::member(U x){return x+4;}
template<class T> int Box<T>::Inner::data=5;
int main(){Box<int>::Inner x;int (Box<int>::Inner::*ptr)(int)const=&Box<int>::Inner::method;
return x.value(1)!=2 || x.value(1L)!=3 || x.inside(1)!=4 || (x.*ptr)(1)!=6 || x.member(1)!=5;}
