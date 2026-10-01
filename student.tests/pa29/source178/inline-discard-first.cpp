int calls;
template<class T> int init() { ++calls; return 17; }
template<class T> struct Box { inline static int values[]={init<T>()}; };
int main() {
  if constexpr(false) return sizeof(Box<int>::values);
  return Box<int>::values[0]==17 && calls==1 ? 0 : 1;
}
