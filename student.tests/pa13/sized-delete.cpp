void operator delete(void*) noexcept;
int destroyed;
unsigned long released;
struct Base { virtual ~Base() noexcept { destroyed=destroyed*10+1; } };
struct Derived:Base {
  int n;
  Derived():n(7){}
  ~Derived() noexcept override { destroyed=destroyed*10+n; }
  static void operator delete(void* p, unsigned long size) noexcept { released=size; ::operator delete(p); }
};
int main(){ Base* b=new Derived; delete b; return destroyed!=71 || released!=sizeof(Derived); }
