struct A{void f();}; int main(){(void)const_cast<void(A::*)()>(&A::f);}
