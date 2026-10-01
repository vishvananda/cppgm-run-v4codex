int main(){const int a=3;auto f=[]<class T>(T){return &a;};return *f(0);}
