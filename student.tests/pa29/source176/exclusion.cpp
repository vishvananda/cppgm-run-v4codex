#define EX __attribute__((exclude_from_explicit_instantiation))
template<class T> struct Excluded {
  EX int used() const;
  EX int unused() const { return T::missing; }
  EX static int data;
  EX struct Nested { int call() const; };
  int ordinary() const;
};
template<class T> int Excluded<T>::used() const { return 31; }
template<class T> int Excluded<T>::data = 7;
template<class T> int Excluded<T>::Nested::call() const { return 4; }
template<class T> int Excluded<T>::ordinary() const { return 2; }
extern template struct Excluded<int>;
template struct Excluded<long>;
int main() {
  Excluded<int> a; Excluded<int>::Nested b;
  return a.used() + Excluded<int>::data + b.call() - 42;
}
