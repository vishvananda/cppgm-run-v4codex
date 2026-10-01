int main(){const int v=3;auto f=[]<class T>(T x){return v+x;};return f(4)==7?0:1;}
