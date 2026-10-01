template<class T> auto factory(T x)->decltype(x) {auto f=[x]<class U>(U y){return x+y;};return f(x);}
int main(){return factory(2)==4?0:1;}
