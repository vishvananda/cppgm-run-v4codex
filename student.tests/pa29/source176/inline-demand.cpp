template<class T> struct Lazy {
  inline static int dormant = T::missing;
  inline static const int constant = sizeof(T);
  inline static int selected = 23;
};
static_assert(Lazy<int>::constant == sizeof(int), "constant initializer demand");
int main() { Lazy<int> a; return sizeof(a) + Lazy<int>::selected - 24; }
