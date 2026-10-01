struct S { int a; int b; int c; };
constexpr S global = {.c=9};
static_assert(global.a==0 && global.b==0 && global.c==9, "omitted fields");
int sequence;
int next(){return ++sequence;}
int main(){
 S s={.a=next(),.c=next()}; S t=(S){.b=7,.c=8};
 return s.a!=1 || s.b!=0 || s.c!=2 || sequence!=2 || t.a!=0 || t.b!=7 || t.c!=8;
}
