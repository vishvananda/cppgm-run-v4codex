template<class T> class base { int value; };
template<class T> struct derived : base<T> {
  typedef base<T> alias;
  void reset(derived& other) { other.alias::value = 17; }
};
int main() { derived<int> a; a.reset(a); }
