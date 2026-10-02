struct Enclosing {
  template<class T> struct Nested { void run() { Enclosing value; } };
  Enclosing() = delete;
};
