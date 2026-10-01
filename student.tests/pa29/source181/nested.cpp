int calls;
struct E { E(){++calls;} ~E(){++calls;} };
struct S { int x; E a[1000000][0]; };
static_assert(sizeof(S)==4,"empty dimensions");
int main(){{S s; s.x=7; S other=s; other=s; if(other.x!=7)return 2;}return calls;}
