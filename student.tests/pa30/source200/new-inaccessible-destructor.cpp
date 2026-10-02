struct Hidden { int value; private: ~Hidden() {} };
int main() {
  Hidden* a = new Hidden{7};
  Hidden* b = new Hidden();
  return a->value != 7 || b->value != 0;
}
