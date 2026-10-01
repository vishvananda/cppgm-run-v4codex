struct E { E()=delete; }; struct S { E a[0]; }; int main(){S s;}
