struct B { int x; }; struct D : private B {};
int D::* p = &B::x;
