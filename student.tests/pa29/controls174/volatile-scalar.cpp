template<class... T> void direct(T&... values) { (values, ...); }
volatile int& ref(volatile int& value) { return value; }
template<class... T> void calls(T&... values) { (ref(values), ...); }
int main() { volatile int a=1,b=2,c=3; direct(a,b,c); calls(a,b,c); return 0; }
