template<class T> struct box {
  box() = delete;
  static void use() { box(); }
};
int main() { box<int>::use(); }
