struct Row { int values[3]; int tag; };

template<int Bias>
int visit(int n) {
  Row rows[] = {{{n, Bias}, 1}, {{Bias, n + 1}, 2}, {}};
  auto add = [](int x = Bias) {
    static int calls;
    ++calls;
    int a[2] = {x, calls};
    int sum = 0;
    for (auto v : a) sum += v;
    return sum;
  };
  int (*pointer)(int) = add;
  return rows[0].values[0] + rows[1].tag + rows[2].values[2] + add() + pointer(n);
}

int main() {
  return visit<3>(4) != 16 || visit<5>(4) != 18 || visit<3>(4) != 20;
}
