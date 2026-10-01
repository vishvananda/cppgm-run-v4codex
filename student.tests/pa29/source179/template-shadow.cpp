struct Pair{int a,b;};template<class T>int f(T p){auto [x,y]=p;{auto [x,y]=Pair{7,8};if(x+y!=15)return 1;}return x+y;}
int main(){return f(Pair{2,3})!=5;}
