int calls;
template<class T> int init() { ++calls; return 17; }
template<class T> struct Box { inline static int value=init<T>(); };
int main() {
  if constexpr(false) { static_assert(sizeof(Box<int>)==1, "layout only"); }
  if (int n=Box<int>::value; n!=17) return 1;
  return calls==1 ? 0 : 2;
}
