struct A{~A(){}};
int f(int x){try{A a;if(x)return 1;for(;;){}}catch(...){return 2;}}
int g(){try{A a;throw 3;}catch(...){for(;;){}}}
int main(){return f(1)-1;}
