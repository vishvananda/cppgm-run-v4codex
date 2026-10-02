// [expr.cond]/3,6 and [conv.lval]: class prvalues retain their cv-qualification.
enum Kind { First = 1, Second = 2 };
struct Item { int value; Item(Kind k):value(k){} };
int selected(Item&&) { return 1; }
int selected(const Item&&) { return 2; }
int main(int argc,char**) {
    const Item item(First);
    if (selected(argc == 2 ? Second : item) != 2) return 1;
    if (selected(argc == 2 ? item : Second) != 2) return 2;
    Item result = argc == 2 ? Second : item;
    return result.value != (argc == 2 ? 2 : 1);
}
