template<class...T> int f(T...x){return (x+...);} int main(){return f();}
