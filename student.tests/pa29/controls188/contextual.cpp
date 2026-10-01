// Hosted contextual coroutine syntax is retained without demanding a frame.
template<class T> T coroutine(T t) {
  auto x = co_await t;
  co_yield x;
  co_return x;
}
template<class T> void forms(T t) {
  auto x = co_await t.get();
  auto y = (co_yield t = t);
  co_await co_await t;
  co_yield {x, y};
  co_return {x, y};
}
template<class T> void empty_return(T) { co_return; }
template<class T> void constant_operand(T) { co_yield 7; co_return 8; }
template<class T> struct Deferred {
  void member(T t) { co_await t; co_yield t; co_return; }
  int ordinary() { return 9; }
};
int co_await;
int co_yield;
int co_return;
int ordinary_identifier_use() {
  co_await = 3;
  co_yield = co_await + 4;
  co_return = co_yield * 2;
  return co_return;
}
int main() { Deferred<int> d; return ordinary_identifier_use() == 14 && d.ordinary() == 9 ? 0 : 1; }
