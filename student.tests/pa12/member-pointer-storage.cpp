int copies;
struct Object {
  int n;
  int read(int x) const & { return n+x; }
  int read(int x) && { return n-x; }
};
typedef int (Object::*Read)(int) const &;
struct Tail {
  int n;
  Tail(int x):n(x) {}
  Tail(Tail const& x):n(x.n) { ++copies; }
  Tail& operator=(Tail const& x) { n=x.n; ++copies; return *this; }
};
struct Box {
  Read read;
  Tail tail;
  Box(Read p):read(p),tail(3) {}
};
int main() {
  Read p=&Object::read;
  Read array[2]={p,p};
  Object value={10};
  if((value.*array[1])(2)!=12)return 1;
  Box first(p),second(first);
  first=second;
  if((value.*second.read)(4)!=14 || copies!=2)return 2;
  int (Object::*rvalue)(int) &&=&Object::read;
  return (static_cast<Object&&>(value).*rvalue)(3)!=7;
}
