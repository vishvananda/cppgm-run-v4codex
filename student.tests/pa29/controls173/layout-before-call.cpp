int main(){int a=3,b=5;auto f=[=]<class T>(T x){return a+b+x;};
 auto bytes=sizeof(f);auto g=f;
 if(f(1)!=9 || g(2L)!=10 || sizeof(f)!=bytes || sizeof(g)!=bytes)return 1;
 return 0;}
