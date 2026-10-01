auto select() { if constexpr (true) return 7; else return "unused"; }
template<class T> auto select(T t) { if constexpr (sizeof(T)==sizeof(int)) return t; else return t.absent(); }
int main() { return select()==7 && select(8)==8 ? 0 : 1; }
