struct Bits { mutable unsigned a:3; unsigned b:4; };
int main(){const Bits p{1,2};const auto& [a,b]=p;a=7;static_assert(__is_same(decltype(a),unsigned),"mutable");static_assert(__is_same(decltype(b),const unsigned),"const");return a!=7||b!=2;}
