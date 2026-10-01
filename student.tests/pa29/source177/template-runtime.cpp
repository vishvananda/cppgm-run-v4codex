template<class T> struct Holder { T value; explicit operator bool() const { return value!=0; } };
template<class T> int choose(T x) {
  if (Holder<T> h{x}; h) return h.value;
  else return -1;
}
template<class T> int discard() {
  if constexpr (using Q=T; sizeof(Q)==sizeof(int)) return 7;
  else return Q::missing;
}
int main() { return choose(4)==4 && choose(0)==-1 && discard<int>()==7 ? 0 : 1; }
