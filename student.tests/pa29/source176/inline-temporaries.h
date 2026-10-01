extern int trace;
extern int made;
struct Item {
 int tag;
 Item(int n) : tag(n) { ++made; }
 ~Item() { trace = trace*10+tag; }
};
struct Pair {
 const Item& a;
 const Item& b;
 ~Pair() { trace = trace*10+3; if(a.tag!=1 || b.tag!=2) trace=9; }
};
inline Pair pair{Item(1),Item(2)};
