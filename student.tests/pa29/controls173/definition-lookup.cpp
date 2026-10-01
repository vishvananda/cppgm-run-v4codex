int pick(long){return 1;}
template<class T>int run(T x){return []<class U>(U y){return pick(y);}(x);}
int pick(int){return 2;}
int main(){return run(1)==1?0:1;}
