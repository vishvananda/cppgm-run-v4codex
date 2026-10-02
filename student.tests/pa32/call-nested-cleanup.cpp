int count;
struct Guard { int n; Guard(int x):n(x){} ~Guard(){ count+=n; } };
int leaf(int n){ if(n) throw n; return 7; }
int wrapper(int n){ return leaf(n)+1; }
int protected_call(int n) { Guard a(1); try { Guard b(2); return wrapper(n); }
 catch(int x) { Guard c(4); return x+10; } }
int safe(int n) noexcept { return n+1; }
int nested_safe(int n) { try { try { return safe(n); } catch(...) {return -1;} } catch(...) {return -2;} }
int main(){int a=protected_call(3); int b=protected_call(0);return a!=13||b!=8||count!=10||nested_safe(8)!=9;}
