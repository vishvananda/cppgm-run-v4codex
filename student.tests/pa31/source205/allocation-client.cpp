#include <new>
extern int allocations, releases;
int second();
int main() {
  char* p = new char[9];
  p[8] = 7;
  int value = p[8];
  delete[] p;
  return value == 7 && second() == 11 && allocations == 2 && releases == 2 ? 0 : 1;
}
