struct S {int a[3];int b;};
constexpr int f(){S s={.b=2};s.a[1]=9;S t={.a={1,2,3},.b=4};s=t;s.a[1]=8;S u=s;return u.a[0]+u.a[1]+u.a[2]+u.b;}
static_assert(f()==16,"whole aggregate assignment replaces subobject writes");
int main(){return f()!=16;}
