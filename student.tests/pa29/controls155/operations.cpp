struct Deleted { ~Deleted() = delete; };
struct Hidden { private: ~Hidden(){}; };
struct Throwing { ~Throwing() noexcept(false) {} };
struct Trivial {};
struct Convert { operator int() const noexcept { return 1; } };
struct Explicit { explicit operator int() const { return 1; } };
struct Private { private: operator int() const; };
struct Target { Target(int) noexcept {} };
struct ExplicitTarget { explicit ExplicitTarget(int){} };
using Function = int();
static_assert(__is_destructible(int), "scalar destruction");
static_assert(__is_destructible(int&), "reference destruction");
static_assert(!__is_destructible(void) && !__is_destructible(Function), "nonobjects");
static_assert(!__is_destructible(int[]) && __is_destructible(int[2]), "arrays");
static_assert(!__is_destructible(Deleted) && !__is_trivially_destructible(Deleted), "deleted destructor");
static_assert(!__is_destructible(Hidden), "access");
static_assert(__is_trivially_destructible(Trivial), "trivial");
static_assert(__is_nothrow_destructible(Trivial), "nothrow implicit destructor");
static_assert(__is_destructible(Throwing) && !__is_nothrow_destructible(Throwing), "throwing destructor");
static_assert(__is_convertible(void, const void), "void");
static_assert(!__is_convertible(int, void) && !__is_convertible(void, int), "void nonconversion");
static_assert(__is_convertible(int, long) && __is_nothrow_convertible(int,long), "arithmetic");
static_assert(__is_convertible(Convert,int) && __is_nothrow_convertible(Convert,int), "implicit conversion");
static_assert(!__is_convertible(Explicit,int) && !__is_convertible(Private,int), "explicit and private conversion");
static_assert(__is_convertible(int,Target) && !__is_convertible(int,ExplicitTarget), "copy initialization");
static_assert(__is_convertible(int[2],int*) && !__is_convertible(int*,int[2]), "array conversions");
static_assert(__is_convertible(Function,Function*) && !__is_convertible(Function*,Function), "functions");
static_assert(__is_convertible(int,int&&) && !__is_convertible(int,int&), "categories");
int main(){return 0;}

struct Protected {
 protected: Protected() noexcept {} Protected(const Protected&) noexcept {};
 Protected& operator=(const Protected&) noexcept {return *this;}
};
struct Derived : Protected {};
struct Member { Protected value; };
static_assert(__is_nothrow_constructible(Derived), "base construction in definition context");
static_assert(!__is_constructible(Protected), "direct protected access");
static_assert(!__is_constructible(Member), "member protected access");
static_assert(__is_nothrow_constructible(Derived, const Derived&), "copy definition context");
static_assert(__is_nothrow_assignable(Derived&, const Derived&), "assignment definition context");
Derived make_derived() { Derived a; Derived b(a); b=a; return b; }
