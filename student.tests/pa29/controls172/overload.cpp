struct A {int n;};
A operator-(A a,A b){return A{a.n*10-b.n};}
template<class... T> A left(T... x){return (... - x);}
template<class... T> A right(T... x){return (x - ...);}
int main(){return left(A{4},A{2},A{1}).n!=379 || right(A{4},A{2},A{1}).n!=21;}
