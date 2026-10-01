int copies;struct Pair{int a,b;Pair(int n=2):a(n),b(n+1){}explicit Pair(const Pair& p):a(p.a),b(p.b){++copies;}};
int main(){Pair p;auto [x,y](p);auto [a,b]{p};return copies!=2||a+b+x+y!=10;}
