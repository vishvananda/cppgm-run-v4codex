template<class T> struct Box {
  struct Member { __attribute__((abi_tag("kept"))) static int value(); };
};
template<class T> int Box<T>::Member::value() { return 7; }
int main() { return Box<int>::Member::value()-7; }
