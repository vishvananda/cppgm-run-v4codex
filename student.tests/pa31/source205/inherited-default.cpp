struct Base { int n; Base() noexcept : n(11) {} Base(int x) : n(x) {} };
struct Moved : Base { using Base::Base; Moved(Moved&&) = default; };
struct Member { int n; Member() : n(19) {} };
struct WithMember : Base { using Base::Base; WithMember(int,int) : Base(1) {} Member other; };
struct Local : Base { using Base::Base; Local() : Base(29) {} };
template<class T> struct Wrapped : T { using T::T; Wrapped(Wrapped&&) = default; };
struct Throws { Throws() { throw 7; } };
struct Throwing : Base { using Base::Base; Throwing(Throwing&&) = default; Throws t; };
static_assert(__is_constructible(Moved), "inherited zero arity");
static_assert(__is_constructible(Wrapped<Base>), "dependent inherited zero arity");
int main() {
 Moved a; Wrapped<Base> b; WithMember c; Local d;
 if(a.n!=11 || b.n!=11 || c.n!=11 || c.other.n!=19 || d.n!=29) return 1;
 try { Throwing bad; return 2; } catch(int v) { if(v!=7) return 3; }
 return 0;
}
