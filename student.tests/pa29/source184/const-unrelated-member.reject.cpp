struct A{int x;}; struct B{int x;}; int main(){(void)const_cast<int B::*>(&A::x);}
