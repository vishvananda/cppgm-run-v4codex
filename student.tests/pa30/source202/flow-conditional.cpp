int a(){while(1 ? 2 : 0){}}
int b(){while(1==1 ? (0 ? 0 : 3) : 0){}}
template<class T> int loop(){for(;sizeof(T)==4 ? 2 : 0;){}}
int demanded(){return loop<int>();}
int calls;
int tick(int x){++calls;return x;}
struct Flag{operator bool()const{return true;}};
int f(int x){if(x ? tick(7) : tick(0))return 11;return 13;}
int g(){if(Flag() ? 5 : 0)return 17;return 18;}
int main(){return f(1)==11 && f(0)==13 && calls==2 && g()==17 ? 0:1;}
