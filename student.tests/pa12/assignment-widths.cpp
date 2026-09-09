struct Value { long n; void clear() { n=0; } void set() { n=7; } };
void assign(Value& x,long& y) { x.n=0; x.n=7; y=0; y=7; }
int main() { Value x={}; x.n=0; x.n=7; long y; y=0; y=7; assign(x,y); return x.n+y!=14; }
