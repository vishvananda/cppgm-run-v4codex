struct A{int x;};struct B{A a;};int main(){return ((B){.a=1}).a.x;}
