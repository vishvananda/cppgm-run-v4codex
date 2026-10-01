#define TAG __attribute__((abi_tag("keep")))
template<class T> TAG int free_fn(T);
template<class T> int free_fn(T x){return x+1;}
struct Plain { template<class U> TAG static int member(U); };
template<class U> int Plain::member(U x){return x+2;}
template<class T> struct Box {
 template<class U> TAG static int member(U);
 struct Inner { template<class U> TAG static int member(U); };
 TAG static int data;
};
template<class T> template<class U> int Box<T>::member(U x){return x+3;}
template<class T> template<class U> int Box<T>::Inner::member(U x){return x+4;}
template<class T> int Box<T>::data=5;
int main(){return free_fn(1)+Plain::member(1)+Box<int>::member(1)+Box<int>::Inner::member(1)+Box<int>::data-19;}
