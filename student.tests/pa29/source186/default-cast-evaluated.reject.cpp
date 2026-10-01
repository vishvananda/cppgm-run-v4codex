template<class T> int f(T p,int n=static_cast<decltype(p)>(p)){return n;} int main(){return f(2);}
