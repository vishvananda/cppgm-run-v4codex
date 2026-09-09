class Base { Base(int) {} };
struct Derived : Base { using Base::Base; };
int main() { Derived d(1); }
