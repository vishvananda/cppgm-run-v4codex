int value=11;
int conversions=0;
struct Both {
 operator int&() const {++conversions; return value;}
 operator int() const {conversions+=100; return 5;}
};
struct Explicit { explicit operator int() const {++conversions; return 17;} };
struct ExplicitRef { explicit operator int&() const {++conversions;return value;} };
struct RefQualified {
 operator int&() & {++conversions;return value;}
 operator int() && {++conversions;return 23;}
};
static_assert(!__reference_constructs_from_temporary(const int&,Both),"lvalue binding precedes rvalue conversion");
static_assert(__reference_constructs_from_temporary(const int&,Explicit),"explicit direct");
static_assert(!__reference_converts_from_temporary(const int&,Explicit),"copy excludes explicit");
static_assert(__is_constructible(const int&,Explicit),"constructible shares selection");
static_assert(!__is_convertible(Explicit,const int&),"convertible shares selection");
static_assert(!__reference_constructs_from_temporary(const int&,RefQualified&),"lvalue-qualified");
static_assert(__reference_constructs_from_temporary(const int&,RefQualified),"rvalue-qualified");
template<class T> int direct(const T& x){const int& r(x);return r;}
int main() {
 Both b; const int& a(b); if (&a!=&value || conversions!=1) return 1;
 Explicit e; const int& c(e); if(c!=17 || conversions!=2) return 2;
 int&& d(e); if(d!=17 || conversions!=3) return 3;
 ExplicitRef r; int& f(r); if(&f!=&value || conversions!=4) return 4;
 if(direct(e)!=17 || conversions!=5) return 5;
 RefQualified q; const int& g(q); if(&g!=&value || conversions!=6) return 6;
 const int& h(static_cast<RefQualified&&>(q)); if(h!=23 || conversions!=7) return 7;
 return 0;
}
