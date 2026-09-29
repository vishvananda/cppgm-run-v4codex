struct A { int pad; }; struct B { int b; };
struct D:A,B { int z; int get()const{return z;} };
int invoke(B& b,int(B::*p)()const){return (b.*p)();}
int main() {
 D d; d.pad=11;d.b=22;d.z=33;
 int(D::*whole)()const=&D::get;
 int(B::*back)()const=static_cast<int(B::*)()const>(whole);
 if((d.*back)()!=33 || invoke(d,back)!=33) return 1;
 int(B::*null)()const=0;
 if(back==null || !back) return 2;
 return 0;
}
