struct X{int x;};
struct Safe { bool value; operator int X::*()const{return value ? &X::x : 0;} };
int main(){Safe a={true},b={false};bool x=a,y=b;
 if(!x || y || !a || b) return 1;
 return (a ? 0 : 2) || (b ? 3 : 0);}
