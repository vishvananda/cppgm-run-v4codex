int unavailable();
template<class T> int dormant() { return T::missing; }
int main() {
  if constexpr (false) { dormant<int>(); return unavailable(); }
  return 0;
}
