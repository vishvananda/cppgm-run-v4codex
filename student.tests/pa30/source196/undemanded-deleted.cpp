template<class T> struct box {
  box() = delete;
  static void unused() { box(); }
};
int main() { return sizeof(box<int>) == 1 ? 0 : 1; }
