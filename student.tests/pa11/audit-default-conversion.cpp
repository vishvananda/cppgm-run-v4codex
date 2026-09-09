int alive;
struct Box{int x;Box(int n):x(n+2){++alive;}~Box(){--alive;}};
struct A{int x;A(const Box& b=5):x(b.x){if(alive!=1)x=99;}};
A globals[2];
int main(){A a; if(alive || a.x!=7 || globals[1].x!=7)return 1; A b[12]; A c[12]={}; return alive || b[11].x!=7 || c[11].x!=7;}
