class B { B() {} public: B(int) {} };
struct D : B { using B::B; D(D&&) = default; };
D d;
