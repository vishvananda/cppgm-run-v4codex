template<class T> struct incomplete;
template<class T> struct owner {
  static int dormant() { return incomplete<T>::value; }
};
int main() { return owner<int>::dormant(); }
