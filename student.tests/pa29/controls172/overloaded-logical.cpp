int seen;
struct A {int n;};
A make(int n){seen=seen*10+n;return A{n};}
A operator&&(A a,A b){return A{a.n+b.n};}
template<class...T> A sum(T...x){return (make(x)&&...);}
int main(){A a=sum(1,0,3);return a.n!=4 || seen!=103;}
