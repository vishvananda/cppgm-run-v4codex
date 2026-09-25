// One bound fact: pack expansion -> initializer plan -> query -> deduction -> copy.
template<class T, unsigned long N>
int rows(T (&)[N]) { return N; }

template<int... Values>
int lookup(unsigned index) {
  long table[][2] = {Values...};
  static_assert(sizeof(table) == 4 * sizeof(long), "deduced row count");
  decltype(table) independent = {};
  independent[index][1] = 17;
  return rows(table) == 2 && table[index][1] != independent[index][1]
      ? table[index][1] : -1;
}

int main() {
  volatile unsigned index = 1;
  return lookup<1, 2, 3, 4>(index) != 4;
}
