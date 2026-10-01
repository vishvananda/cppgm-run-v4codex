template<class T> int width(int n=sizeof(T));
int before() { return width<long>(); }
template<class U> int width(int n) { return n; }
int main() { return before()==sizeof(long) && width<char>()==sizeof(char) ? 0 : 1; }
