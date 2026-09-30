extern "C" int imported(int n) { return n*2+2; }
extern "C" int (*host_address())(int) { return imported; }
extern "C" int call_pointer(int (*p)(int),int n) { return p(n); }
