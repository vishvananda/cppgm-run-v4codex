int add(int a, int b) { return a+b; }
int identity(int a) { return a; }
typedef int (*Binary)(int,int);
typedef int (*Unary)(int);
struct Base { operator Binary() const { return add; } };
struct Callable : Base {
  int operator()(int x) { return x+1; }
  operator Unary() const { return identity; }
};
struct Explicit { explicit operator Binary() const { return add; } };
int main() {
  Callable f;
  if (f(5)!=6 || f(2,3)!=5) return 1;
  const Callable c;
  if (c(5)!=5 || c(3,4)!=7) return 2;
  Explicit e;
  Binary pointer(e);
  return pointer(1,2)-3;
}
