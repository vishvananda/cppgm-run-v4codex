int main(){auto f=[]<class T>(T t){return t;};long(*p)(int)=f;return p(1);}
