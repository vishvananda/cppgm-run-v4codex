template<class... T> int sum(T... xs){auto f=[=]<class U>(U u){return (xs+...+u);};return f(4);}
int main(){return sum(1,2,3)==10 && sum()==4 ? 0:1;}
