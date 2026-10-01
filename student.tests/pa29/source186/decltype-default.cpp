template<class T> int f(T p,int n=decltype(p)(3)){return n;}int main(){return f(1)!=3;}
