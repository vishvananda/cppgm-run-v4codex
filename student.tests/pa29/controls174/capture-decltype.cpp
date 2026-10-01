int copied;
struct A { A() {} A(const A&) { ++copied; } };
int main() {
    A object;
    auto f = [=]<class T>(T value) { decltype(object) local; return value; };
    if (copied) return 1;
    return f(3) == 3 && copied == 0 ? 0 : 2;
}
