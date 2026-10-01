struct Small { long a; double b; };
struct Large { long a,b,c; };
int call_int(int (^block)(int),int x) { return block(x); }
double call_float(double (^block)(float,double),float x,double y) {return block(x,y);}
Small call_small(Small (^block)(Small),Small value) {return block(value);}
Large call_large(Large (^block)(Large),Large value) {return block(value);}
int& call_reference(int& (^block)(int&),int& x) {return block(x);}
int call_variadic(int (^block)(int,...),int x) {return block(x,2.5,4);}
void call_void(void (^block)(int&),int& x) { block(x); }
int call_throw(int (^block)(int),int x) {try {return block(x);}catch(int n){return n+1;}}
typedef int (^Callback)(int);
Callback return_block(Callback block){return block;}
void throw_block(Callback block){throw block;}
int catch_block(void (*raise)(Callback),Callback block){try{raise(block);}catch(Callback caught){return caught(3);}return -1;}
int self_catch(Callback block){try{throw block;}catch(Callback b){return b(4);}}
