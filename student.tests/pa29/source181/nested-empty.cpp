struct E{}; struct Z { E a[0]; }; struct S { Z z[2]; E e; }; static_assert(sizeof(S)==1,"size");int main(){S s;}
