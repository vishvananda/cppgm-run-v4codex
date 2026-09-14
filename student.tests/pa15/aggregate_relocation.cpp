// An aggregate array element is initialized at its final address [dcl.init.aggr].
// A bitwise copy from a scratch array cannot replace this initialization.
struct Item {
  Item* self;
  Item(int) : self(this) {}
  Item(const Item&) : self(this) {}
  Item(Item&&) : self(this) {}
};
template<class T, int N> struct Array { T elements[N]; };
int inspect(Array<Item, 2>&& a) {
  return a.elements[0].self == &a.elements[0] &&
         a.elements[1].self == &a.elements[1] ? 0 : 1;
}
template<class T> int test() { return inspect(T{Item(0), Item(1)}); }
int main() { return test<Array<Item, 2>>(); }
