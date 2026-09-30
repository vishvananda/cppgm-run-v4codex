extern "C" int printf(const char*, ...);
struct VRoot {
  VRoot(); virtual ~VRoot();
  virtual long value(); virtual VRoot* address();
};
struct VOwner : virtual VRoot {
  VOwner(); virtual ~VOwner();
  virtual VRoot* self();
  virtual VRoot& ref();
};
struct Padding153 { long words[5]; };
struct VDerived : VOwner, Padding153 {
  VDerived(); ~VDerived();
  VDerived* self(); VDerived& ref(); long value();
};
struct VMore : VDerived { long extra[7]; VMore(); ~VMore(); };
struct Left153 : virtual VRoot { int left; Left153(); ~Left153(); long value(); };
struct Right153 : virtual VRoot { int right; Right153(); ~Right153(); long value(); };
struct Diamond153 : Left153, Right153 {
  int data; Diamond153(); ~Diamond153(); long value();
};
long producer_size(int);
void* create153(int);
void destroy153(int,void*);
void observe153(int,VRoot*,long);
struct Nested153 : virtual VOwner {
  Nested153(); ~Nested153(); long value();
};
struct Nonempty153 : virtual VRoot {
  int member; Nonempty153(); ~Nonempty153(); long value();
};
struct Indirect153 : virtual Nonempty153 {
  Indirect153(); ~Indirect153(); long value();
};
struct Another153 { virtual ~Another153(); virtual long other(); };
struct Mixed153 : VOwner, virtual Another153 {
  int member; Mixed153(); ~Mixed153(); long value(); long other();
};
struct Multi153 : VRoot, Another153 {
  int member; Multi153(); ~Multi153(); long value(); long other();
};
struct VirtualMulti153 : virtual Multi153 {
  int extra; VirtualMulti153(); ~VirtualMulti153(); long value(); long other();
};
struct InlineKey153 : VOwner, virtual Another153 {
  InlineKey153() {} virtual long key();
};
struct Choice153 : virtual Nonempty153, virtual VOwner {
  Choice153(); ~Choice153(); long value();
};
struct alignas(32) Aligned153 { virtual ~Aligned153(); };
struct AlignOwner153 : virtual Aligned153 { int data; };
