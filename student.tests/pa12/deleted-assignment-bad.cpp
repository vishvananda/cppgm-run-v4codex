struct X {
  X& operator=(const X&) { return *this; }
  X& operator=(X&&) = delete;
};
int main() { X a; X b; a = static_cast<X&&>(b); }
