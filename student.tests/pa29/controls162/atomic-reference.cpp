void set(_Atomic(int)& x){x=9;}
int main(){_Atomic(int) x=3;set(x);return x==9?0:1;}
