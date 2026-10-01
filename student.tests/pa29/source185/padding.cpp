using S = _BitInt(7);
using U = unsigned _BitInt(7);
S identity(S x) { return x; }
int main() {
 S x=0; U y=0;
 *reinterpret_cast<unsigned char*>(&x)=127;
 *reinterpret_cast<unsigned char*>(&y)=255;
 if(long(x)!=-1 || long(y)!=127 || identity(x)!=-1) return 1;
 volatile S z=x;
 return z==-1 ? 0 : 2;
}
