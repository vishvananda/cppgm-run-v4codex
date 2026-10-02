int live, destroyed;
struct Item {
  int value;
  Item(int v) : value(v) { if(v==9) throw v; ++live; }
  ~Item() { --live; ++destroyed; }
};
struct Aggregate { Item a; Item b; };
union Choice { Aggregate aggregate; int unused; ~Choice() {} };
struct Owner {
  Choice choice;
  Owner(int v) : choice{{3,v}} {}
  ~Owner() { choice.aggregate.~Aggregate(); }
};
int main() {
  try { Owner owner(9); return 1; } catch(int x) { if(x!=9) return 2; }
  return live!=0 || destroyed!=1;
}
