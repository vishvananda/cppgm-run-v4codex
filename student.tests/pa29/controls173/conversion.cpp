int apply(int(*f)(int),int x){return f(x);}
int main(){
 auto f=[]<class T>(T x){return x+1;};
 int(*p)(int)=f;
 long(*q)(long)=f;
 if(p(3)!=4 || q(7)!=8 || apply(f,10)!=11)return 1;
 return 0;
}
