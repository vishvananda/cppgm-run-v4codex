struct Plain { __attribute__((abi_tag("keep"))) static int value(int x); };
int use(int x){return Plain::value(x);}
