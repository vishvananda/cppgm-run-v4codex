// Reduced from FactStore::operator[] during PA34 seed compilation.
// The declaration's selected implicit constructor needs its completed actions
// before static initialization can be classified. C++11 [stmt.dcl], [class.ctor].
struct fact { unsigned value = 7; };
struct store {
    const fact& get() const { static const fact empty; return empty; }
};
int calls;
int next() { return ++calls; }
struct dynamic { int value = next(); };
const dynamic& get_dynamic() { static const dynamic value; return value; }
struct plain { unsigned value; };
const plain& get_plain() { static plain value; return value; }
int main() {
    store s;
    if (s.get().value != 7 || &s.get() != &s.get()) return 1;
    if (get_dynamic().value != 1 || get_dynamic().value != 1 || calls != 1) return 2;
    return get_plain().value != 0;
}
