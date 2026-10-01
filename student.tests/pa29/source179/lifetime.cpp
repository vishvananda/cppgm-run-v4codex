int live, copies, destroyed;
struct Pair { int a,b; Pair(int v):a(v),b(v+1){++live;} Pair(const Pair& p):a(p.a),b(p.b){++live;++copies;} ~Pair(){--live;++destroyed;} };
int normal(){Pair p(2); {auto [a,b]=p; if(live!=2 || a+b!=5)return 1;} if(live!=1 || copies!=1)return 2; {auto&& [a,b]=Pair(4); if(live!=2 || a+b!=9)return 3;} return live!=1;}
int early(){auto [a,b]=Pair(6); return a+b;}
int main(){if(normal() || live)return 1;if(early()!=13 || live)return 2;try{auto [a,b]=Pair(2);throw a+b;}catch(int x){if(x!=5||live)return 3;}return 0;}
