int calls;
struct Number {
  int value;
  Number(int n) : value(n) {}
  operator int() & { ++calls; return value; }
  operator long() const { ++calls; return value+10; }
};
int rank(int) { return 1; }
int rank(long) { return 2; }
struct Pointer {
  int* p;
  Pointer(int* v) : p(v) {}
  operator int*() const { return p; }
};
struct Explicit {
  explicit operator bool() const { return true; }
};
struct Ref {
  int value;
  operator int&() { return value; }
};
struct Base { operator short() const { return 8; } };
struct Derived : Base { operator int() const { return 9; } };
int main() {
  Number n(4); const Number c(5);
  if (rank(n)!=1 || rank(c)!=2 || calls!=2) return 1;
  if (n+3!=7 || 3+n!=7 || n<2 || c-10!=5) return 2;
  int a[3]={3,5,7}; Pointer p(a);
  if (p[1]!=5 || *(p+2)!=7 || p-a!=0 || p==nullptr) return 3;
  Explicit x; bool b(x); bool d=static_cast<bool>(x);
  if (!x || !(x&&b) || !d) return 4;
  if (((x))) { if (!(x ? 1 : 0)) return 7; } else return 8;
  int left=1, right=2;
  (x ? left : right);
  Ref r; r.value=10; int& v=r; v=11;
  if (r+1!=12 || r.value!=11) return 5;
  Derived derived; int exact=derived; short inherited=derived;
  if (exact!=9 || inherited!=8) return 6;
  return 0;
}
