struct B { B() {} };
struct Bad { Bad() = delete; };
struct D : B { using B::B; D(D&&) = default; Bad bad; };
D d;
