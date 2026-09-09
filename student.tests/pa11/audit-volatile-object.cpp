struct A{int x;};
struct B{A a[2];};
int main(){A ordinary[1]={{1}}; volatile A a[1]={{3}}; volatile B b={{{4},{5}}}; return ordinary[0].x-1+a[0].x-3+b.a[0].x-4+b.a[1].x-5;}
