struct Enclosing {
  template<class T> struct Nested {
    int run() { Enclosing value; return value.count; }
  };
  int count = 7;
};
int main() { Enclosing::Nested<int> n; return n.run()-7; }
