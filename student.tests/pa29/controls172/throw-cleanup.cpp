int alive;
struct A{int n;A(int x):n(x){++alive;}A(const A&a):n(a.n){++alive;}~A(){--alive;}explicit operator bool()const{return n!=0;}};
A make(int n){if(n==2)throw 7;return A(n);}
template<class...T> bool test(T...x){return (make(x)&&...);}
int main(){try{test(1,2,3);}catch(int x){return alive || x!=7;}return 2;}
