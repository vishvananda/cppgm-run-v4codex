void enter(int);
void leave(int);
struct B { B() { enter(2); } ~B() { leave(2); } } b;
