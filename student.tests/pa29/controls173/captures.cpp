struct Value { int n; int run() { auto f=[this]<class T>(T x){return n+x;};return f(3); } };
int main(){
 int a=3,b=9;
 auto bycopy=[=]<class T>(T x) mutable { a+=x; return a; };
 a=20;
 if(bycopy(4)!=7 || bycopy(5L)!=12 || a!=20) return 1;
 auto byref=[&]<class T>(T x){a+=x;return a;};
 if(byref(2)!=22 || byref(3L)!=25) return 2;
 auto mixed=[a,&b]<class T>(T x){b+=x;return a+b;};
 if(mixed(1)!=35 || b!=10) return 3;
 auto nested=[=]<class T>(T x){return [=]<class U>(U y){return a+x+y;}(2);};
 if(nested(3)!=30) return 4;
 auto ordinary=[&](){return [&]<class T>(T x){return a+x;}(2);};
 if(ordinary()!=27) return 5;
 Value v={6}; if(v.run()!=9) return 6;
 return 0;
}
