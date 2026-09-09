int copies;
struct Value { int n; Value(int x):n(x){} Value(const Value& x):n(x.n){++copies;} };
struct Holder { Value value; };
const Holder first={Value(3)}, second={Value(4)};
const Holder table[]={first,second};
struct Ref { const int& r; Value value; };
struct Pair { Ref left; int right; };
const Ref& read(const Ref& x) { return x; }
int run(int n) {
  int before=copies;
  Ref x={n,Value(n+1)};
  Pair y={read(x),7};
  if(copies!=before+1 || &y.left.r!=&n || y.left.value.n!=n+1 ||
     y.right!=7 || table[0].value.n!=3 || table[1].value.n!=4)
    __builtin_abort();
  return y.left.value.n;
}
int main(){if(copies!=2)return 1;return run(7)!=8||copies!=3;}
