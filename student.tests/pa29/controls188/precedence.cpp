template<class T> void precedence(T t) {
  auto a = co_await t.member() + 1;
  auto b = (co_yield t = t);
  auto c = (co_yield t ? t : t);
  auto d = (co_yield t, t);
  co_return t + 1;
}
int main() { return 0; }
