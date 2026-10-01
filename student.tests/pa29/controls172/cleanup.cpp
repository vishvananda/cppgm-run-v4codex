int alive, seen;
struct A{
 int n;
 A(int x):n(x){++alive;}
 A(const A& a):n(a.n){++alive;}
 ~A(){--alive;}
 explicit operator bool() const {seen=alive;return n!=0;}
};
A make(int n){return A(n);}
template<class...T> bool test(T...x){return (make(x)&&...);}
int main(){
 if(test(1,0,3) || alive || seen!=2)return 1;
 if(!test(1,2,3) || alive || seen!=3)return 2;
 return 0;
}
