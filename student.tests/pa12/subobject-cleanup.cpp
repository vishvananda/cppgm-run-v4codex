int trace;
struct Leaf {
  int number;
  Leaf(int n) noexcept:number(n) {}
  ~Leaf() noexcept(false) { trace=trace*10+number; }
};
struct Small {
  Leaf a,b,c;
  Small() noexcept:a(1),b(2),c(3) {}
};
struct Large {
  Leaf a,b,c,d,e,f,g,h,i;
  Large() noexcept:a(1),b(2),c(3),d(4),e(5),f(6),g(7),h(8),i(9) {}
};
int main() {
  { Small value; }
  if(trace!=321)return 1;
  trace=0;
  { Large value; }
  return trace!=987654321;
}
