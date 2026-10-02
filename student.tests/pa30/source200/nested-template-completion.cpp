template<class T> struct Enclosing {
  template<class U> struct Nested {
    template<class V> int run() { Enclosing value; return value.count; }
  };
  int count = 9;
};
int main() { Enclosing<int>::Nested<char> n; return n.run<double>()-9; }
