// PA16 README automatic scalar-array copy rule; N3485 [basic.types]/3,9.
int read(int i) {
  int a[3] = {1, 2};
  int b[3] = {1, 2};
  a[i] = 9;
  return a != b && b[0] == 1 && b[1] == 2 && b[2] == 0 && a[i] == 9 ? 0 : 1;
}
int main() { return read(0) || read(2); }
