// N3485 [basic.start.init]/2, [basic.start.term]/1; PA8 singleton runtime hooks.
int count;
int order[2];
void enter(int value) { order[count++] = value; }
void leave(int value) { if (order[--count] != value) __builtin_abort(); }
struct A { A() { enter(1); } ~A() { leave(1); } } a;
int main() { return count == 2 ? 0 : 1; }
