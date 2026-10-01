template<class T> int width() { return sizeof(T); }
template<class T> int read(int n=width<T>());
template<class U> int read(int n) { return n; }
template<class T> struct Outer {
  template<class U> static int read(int n=width<T>()+width<U>()) { return n; }
};
int main() {
  return read<int>()==sizeof(int) && read<long>()==sizeof(long) &&
    Outer<char>::read<long>()==sizeof(char)+sizeof(long) ? 0 : 1;
}
