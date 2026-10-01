struct unrelated { int value; };
template<class T> struct derived {
  typedef unrelated alias;
  void reset(derived& other) { other.alias::value = 17; }
};
int main() { derived<int> a; a.reset(a); }
