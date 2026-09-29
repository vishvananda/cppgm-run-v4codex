int calls;
template<int = 0> struct item {
  static const int result = 0;
  friend void operator>(int, const item&) { ++calls; }
};
struct initial { item<> objects[2]; };
template<int> struct select;
template<> struct select<sizeof(item<>)> { static const int category = 0; };
template<> struct select<sizeof(initial)> {
  template<int> struct category { static const int result = 0; };
};
extern initial value;
template<class T> void use() {
  item<select<sizeof(value)>::category<0>::result> value;
  { item<select<sizeof(value)>::category<0>::result> value; }
}
int main() {
  use<int>(); use<long>(); use<int>(); use<long>();
  return calls != 4;
}
