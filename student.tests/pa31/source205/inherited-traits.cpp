struct B { B() noexcept {} B(int) {} };
struct Bad { Bad() = delete; };
struct D : B { using B::B; D(D&&) = default; Bad bad; };
struct Throws { Throws() {} };
struct T : B { using B::B; T(T&&) = default; Throws t; };
static_assert(!__is_constructible(D), "deleted inherited member");
static_assert(!__is_nothrow_constructible(T), "throwing inherited member");
int main() {}
struct Private { private: Private() {} };
struct Inaccessible : B { using B::B; Inaccessible(Inaccessible&&) = default; Private member; };
static_assert(!__is_constructible(Inaccessible), "inaccessible extra member");
struct Ref : B { using B::B; Ref(Ref&&) = default; int& ref; };
static_assert(!__is_constructible(Ref), "uninitialized reference member");
struct Explicit : B { using B::B; Explicit(int,int) : B() {} int& ref = global; static int global; };
static_assert(__is_constructible(Explicit), "initialized reference member");
struct Param : B { using B::B; Bad bad; };
static_assert(!__is_constructible(Param,int), "parameterized inherited member");
