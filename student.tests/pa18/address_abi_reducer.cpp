// ABI-only reducer: N3485 [temp.arg.nontype]/1,5 and [temp.type]/1.
struct C { int m; };
namespace ns {
template<int C::*P> struct Holder { static void f(C&) {} };
}
void use(C& c) { ns::Holder<&C::m>::f(c); }
