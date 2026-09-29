struct S {int f(){return 1;}};
int(S::*p)()const=&S::f;
