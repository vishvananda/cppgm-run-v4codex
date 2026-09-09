int effects; int bump(){return ++effects;}
struct A { int x; A(int unused):x(7){} };
A a[2]={{bump()},{bump()}};
struct Empty { Empty(int unused){} };
Empty b[2]={{bump()},{bump()}};
int main(){return effects!=4 || a[1].x!=7;}
