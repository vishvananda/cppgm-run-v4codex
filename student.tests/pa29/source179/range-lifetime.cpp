int live,copies,destroyed;
struct Pair{int a,b;Pair(int n=1):a(n),b(n+1){++live;}Pair(const Pair& p):a(p.a),b(p.b){++live;++copies;}~Pair(){--live;++destroyed;}};
int f(){Pair p[3];int before=destroyed;for(auto [a,b]:p){if(live!=4)return 1;continue;}if(live!=3||copies!=3||destroyed-before!=3)return 2;
 try{for(auto [a,b]:p){throw a+b;}}catch(int x){if(x!=3||live!=3)return 3;}for(auto [a,b]:p){break;}return live!=3;}
int main(){return f()||live;}
