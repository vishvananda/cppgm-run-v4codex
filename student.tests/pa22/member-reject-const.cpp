struct S { int f(){return 1;} };
int main(){ const S s; int(S::*p)()=&S::f; return (s.*p)(); }
