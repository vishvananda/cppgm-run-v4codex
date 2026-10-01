struct Pair { int first; int& second; };
Pair make(int& x) { return Pair{3,x}; }
int main() {
 int x=2; auto [a,b]=make(x); b=8; a=7;
 static_assert(__is_same(decltype(a),int),"value type");
 static_assert(__is_same(decltype(b),int&),"reference member");
 static_assert(__is_same(decltype((a)),int&),"expression type");
 Pair p{1,x}; auto& [c,d]=p; c=4; d=5;
 const auto& [e,f]=p; f=9;
 static_assert(__is_same(decltype(e),const int),"cv");
 static_assert(__is_same(decltype(f),int&),"reference cv");
 return a!=7 || b!=9 || c!=4 || e!=4 || x!=9;
}
