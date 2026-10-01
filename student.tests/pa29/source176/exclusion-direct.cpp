template<class T> struct Owner {
  __attribute__((exclude_from_explicit_instantiation)) int used();
  __attribute__((exclude_from_explicit_instantiation)) int dormant();
};
template<class T> int Owner<T>::used() { return 9; }
template<class T> int Owner<T>::dormant() { return 3; }
extern template struct Owner<int>;
template int Owner<int>::used();
extern template int Owner<long>::dormant();
int main() { Owner<int> a; return a.used()-9; }
