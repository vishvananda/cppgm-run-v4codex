int copies, moves, assigns;
struct Item {
  int value;
  Item(): value(7) {}
  Item(const Item& x): value(x.value) { ++copies; }
  Item(Item&& x): value(x.value) { ++moves; x.value = 0; }
  Item& operator=(const Item& x) { value=x.value; ++assigns; return *this; }
};
struct Small { Item items[2][2]; };
struct Large { Item items[3][3]; };
int main() {
  Small a; Small b(a); Small c(static_cast<Small&&>(a)); b=c;
  Large x; Large y(x); Large z(static_cast<Large&&>(x)); y=z;
  return copies!=13 || moves!=13 || assigns!=13 || x.items[2][2].value!=0 || y.items[2][2].value!=7;
}
