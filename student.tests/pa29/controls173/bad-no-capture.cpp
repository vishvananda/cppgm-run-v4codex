int main(){int x=2;auto f=[]<class T>(T t){return x+t;};return f(1);}
