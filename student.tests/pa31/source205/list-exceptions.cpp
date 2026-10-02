#include <initializer_list>
int live = 0, destroyed = 0;
struct Item {
  int n;
  Item(int v) noexcept : n(v) { ++live; }
  Item(const Item& v) noexcept : n(v.n) { ++live; }
  ~Item() noexcept { --live; ++destroyed; }
};
struct Box {
  int sum;
  Box(std::initializer_list<Item> xs) noexcept : sum(0) { for (const Item& x : xs) sum += x.n; }
};
int consume(const Box& b) noexcept { return b.sum; }
int fail() { throw 9; }
struct Pair { int a, b; };
struct Pairs { Pairs(std::initializer_list<Pair>) noexcept {} };
void pairs(const Pairs&) noexcept {}
struct ThrowDtor { ~ThrowDtor() noexcept(false) {} };
struct Dtors { Dtors(std::initializer_list<ThrowDtor>) noexcept {} };
void dtors(const Dtors&) noexcept {}
struct ThrowCtor { ThrowCtor(std::initializer_list<int>) {} };
void ctor(const ThrowCtor&) noexcept {}
static_assert(noexcept(consume({1,2})), "nonthrowing list");
static_assert(!noexcept(consume({fail()})), "throwing element");
static_assert(!noexcept(pairs({{fail(),2}})), "throwing aggregate element");
static_assert(!noexcept(dtors({{}})), "throwing element destruction");
static_assert(!noexcept(ctor({1})), "throwing list constructor");
int main() {
  int total = consume({3,4});
  if (total != 7 || live || destroyed != 2) return 1;
  try { consume({1,fail()}); return 2; } catch (int n) { if(n!=9) return 3; }
  return live == 0 && destroyed == 3 ? 0 : 4;
}
