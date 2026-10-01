struct E{int a[0];}; E f(E a){return a;}int main(){E a{}; E b=f(a);return sizeof(b);}
