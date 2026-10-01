struct Has {int value;};
template<class F,class T>auto invoke(F f,T t)->decltype(f(t)){return f(t);}
int invoke(...){return -1;}
int main(){
 auto f=[]<class T>(T t)->decltype(t.value){return t.value;};
 return invoke(f,Has{7})==7 && invoke(f,1)==-1 ? 0:1;
}
