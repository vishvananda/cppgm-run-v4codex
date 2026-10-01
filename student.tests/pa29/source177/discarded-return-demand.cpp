template<class T> int dormant() { return T::missing; }
template<class T> auto deduced() { return dormant<T>(); }
int main() { if constexpr(false) deduced<int>(); }
