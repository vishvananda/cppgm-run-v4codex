int calls = 0;
int next() { ++calls; if (calls == 2) throw 17; return 9; }
struct Base { int value; Base(int n = next()) noexcept : value(n) {} };
struct Derived : Base { using Base::Base; Derived(Derived&&) = default; };
struct Other { int value; Other() noexcept : value(31) {} };
struct Multiple : Base, Other { using Base::Base; Multiple(Multiple&&) = default; };
struct Deleted { Deleted() = delete; };
struct Invalid : Base, Deleted { using Base::Base; Invalid(Invalid&&) = default; };
static_assert(__is_constructible(Derived), "inherited defaults");
static_assert(!__is_nothrow_constructible(Derived), "throwing inherited default argument");
static_assert(__is_nothrow_constructible(Derived,int), "explicit argument skips default");
static_assert(!__is_constructible(Invalid), "other base must initialize");
static_assert(!__is_constructible(Invalid,int), "parameterized other base must initialize");
int main() {
    Derived a;
    if (a.value != 9 || calls != 1) return 1;
    try { Derived b; return 2; } catch (int n) { if (n != 17) return 3; }
    Multiple c(7);
    return c.Base::value == 7 && c.Other::value == 31 && calls == 2 ? 0 : 4;
}
