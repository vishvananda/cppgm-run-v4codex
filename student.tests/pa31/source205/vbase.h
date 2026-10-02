struct V { int value; V() : value(17) {} virtual ~V() {} };
struct B : virtual V { B() {} virtual ~B() {} int read() const { return value; } };
