int count;
int record(int x) {count=count*10+x;return x;}
template<class... T> bool all(T... x) {return (record(x) && ...);}
template<class... T> bool any(T... x) {return (... || record(x));}
template<class... T> void comma(T... x) { (record(x), ...); }
template<class... T> int& last(T&... x) { return (x,...); }
template<class... T> int assign(int& n,T... x){return (n += ... += x);}
int main(){
 if(all(1,0,3) || count!=10)return 1;
 count=0;if(!any(0,2,3) || count!=2)return 2;
 count=0;comma(1,2,3);if(count!=123)return 3;
 int a=2,b=4;last(a,b)=8;if(b!=8)return 4;
 if(assign(a,3,4)!=9 || a!=9)return 5;
 return 0;
}
