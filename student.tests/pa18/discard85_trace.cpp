int copies, destroyed;
struct value {
  value() {}
  value(volatile value&) { ++copies; }
  ~value() { ++destroyed; }
};
volatile value source;
template<class T> auto inspect(T* p) -> decltype((void)*p, int()) {
  (void)*p;
  return 3;
}
int main() {
  int result = inspect(&source);
  return result != 3 || copies != 1 || destroyed != 1;
}
