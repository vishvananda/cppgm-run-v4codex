int main() {
 int x=7; const int k=3; static int count=2;
 struct Local { int get(int a) { int b=4; return k+count+a+b+sizeof(x); } };
 auto f=[&] { return [=] { return x+k; }(); };
 return Local().get(5)==18 && f()==10 ? 0:1;
}
