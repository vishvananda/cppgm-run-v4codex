template<class... T> int count(int n=sizeof...(T)) { return n; }
template<class T> int width(int n=sizeof(T)) { return n; }
int main() { return count<int,char,long>()==3 && count<>()==0 && width<long>()==sizeof(long) ? 0 : 1; }
