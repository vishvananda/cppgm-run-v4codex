struct B { int x; }; struct L:B {}; struct R:B {}; struct D:L,R {};
int D::* p = &B::x;
