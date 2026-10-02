// C++11 [dcl.spec.auto]/7 requires the same deduced placeholder type for
// every declarator. GCC 15.2 accepts this dependent form; Clang rejects it.
struct record { unsigned id; };
template<class T> unsigned f(const T& r) {
    auto number = r.id, object = r;
    return number + object.id;
}
int main() { return f(record{0}); }
