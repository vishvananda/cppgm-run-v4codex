inline int invoke(int x){return []<class T>(T a){return a+3;}(x);}
inline int (*callback())(int){return []<class T>(T a){return a*2;};}
inline int signatures(int x){
 auto a=[]<class T>(T t)->int{return t+1;};
 auto b=[]<class U>(U u)->int{return u+2;};
 auto c=[]<class...T>(T...t)->int{return (t+...+0);};
 auto d=[]<int...N>(int n)->int{return sizeof...(N)+n;};
 return a(x)+b(x)+c(x,x)+d.operator()<1,2>(x);
}
