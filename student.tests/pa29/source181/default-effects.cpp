int made,dead,defaults;
struct E {E(int x=++defaults){++made;}~E(){++dead;}};
struct S {int n;E a[0];};
int main(){{S s;s.n=4;S t=s;t=s;if(t.n!=4)return 2;}return made||dead||defaults;}
