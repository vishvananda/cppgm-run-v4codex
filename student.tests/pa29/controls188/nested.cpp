template<class T> void nested(T t) {
  auto fn = []<class U>(U u) { auto x = co_await u; co_yield x; co_return x; };
  (void)fn;
  if (true) { co_await t; } else { co_yield t; }
  for (int i=0; i<2; ++i) { co_yield t; }
  try { co_await t; } catch (...) { co_return; }
}
int main() { return 0; }
