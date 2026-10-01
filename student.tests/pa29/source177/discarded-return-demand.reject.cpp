template<class T> auto invalid() { return T::missing; }
int main() { if constexpr(false) invalid<int>(); }
