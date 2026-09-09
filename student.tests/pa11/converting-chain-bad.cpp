struct Inner { Inner(int) {} };
struct Outer { Outer(const Inner&) {} };
void read(const Outer&) {}
int main() { read(1); }
