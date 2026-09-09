struct A {
  A() : A(1) {}
  A(int) : A(1, 2) {}
  A(int, int) : A() {}
};
int main() { A value; }
