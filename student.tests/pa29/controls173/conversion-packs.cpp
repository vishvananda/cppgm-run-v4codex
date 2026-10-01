int global=0;
int main(){
 auto f=[]<class... T>(T...xs)->int{global++;return (xs+...+0);};
 int(*p)(int,int)=f;int(*z)()=f;
 if(p(2,3)!=5 || z()!=0 || global!=2)return 1;
 auto byref=[]<class T>(T* p)->T&{return *p;};
 int&(*ref)(int*)=byref;int value=3;ref(&value)=7;
 return value==7?0:2;
}
