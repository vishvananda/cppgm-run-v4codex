struct Plain { __attribute__((abi_tag("keep"))) static int value(int x); };

int Plain::value(int x){return x+7;}
int use(int x){return Plain::value(x);}
int main(){return use(3)!=10;}
