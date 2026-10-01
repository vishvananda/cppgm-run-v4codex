int copied;
struct A { A() {} A(const A&) { ++copied; } };
template<class T> int work(T value) {
    A object;
    auto outer = [=]<class U>(U arg) {
        auto inner = [=]<class V>(V x) { decltype(object) local; return x; };
        return inner(arg);
    };
    return outer(value);
}
int main() { return work(4) == 4 && copied == 0 ? 0 : 1; }
