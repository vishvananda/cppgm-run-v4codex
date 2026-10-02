int live, destroyed;
struct Item {
  int value;
  Item(int v) : value(v) { ++live; }
  ~Item() { --live; ++destroyed; }
};
struct Aggregate { Item a; Item b; };
union Choice { Aggregate aggregate; int unused; ~Choice() {} };
struct Owner {
  Choice choice;
  Owner(int v) : choice{{v, v+1}} {}
  ~Owner() { choice.aggregate.~Aggregate(); }
};
int main() {
  { Owner owner(3); if(live!=2 || owner.choice.aggregate.b.value!=4) return 1; }
  return live!=0 || destroyed!=2;
}
