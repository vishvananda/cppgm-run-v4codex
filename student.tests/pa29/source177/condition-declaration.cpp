struct Flag { int value; constexpr Flag(int n):value(n){} constexpr explicit operator bool() const { return value!=0; } };
template<int N> int select() {
  if constexpr (constexpr Flag flag{N}) return flag.value;
  else return 9;
}
int main() {
  if constexpr (const int n=2) { if(n!=2)return 1; }
  if constexpr (constexpr bool b=false) return 2;
  else if (b) return 3;
  return select<7>()==7 && select<0>()==9 ? 0 : 4;
}
