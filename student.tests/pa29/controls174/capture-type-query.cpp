int copied;
struct A { A() {} A(const A&) { ++copied; } };
int main() {
    A object;
    auto f = [=]<class T>(T value) {
        typedef __typeof__(object) B;
        B local;
        return value + sizeof(decltype(object));
    };
    return f(2) == 2 + sizeof(A) && copied == 0 ? 0 : 1;
}
