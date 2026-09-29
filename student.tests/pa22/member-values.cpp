struct A { int pad; };
struct B { int x; int get(int y) const { return x+y; } };
struct D : A, B { int z; };
int B::* raw = &B::x;
int D::* data = &B::x;
int (B::* fn)(int) const = &B::get;
int (D::* global_fn)(int) const = &B::get;
constexpr int D::* constant_data = &B::x;
int data_call(D& d, int D::* p) { return d.*p; }
int fn_call(D& d, int (D::* p)(int) const) { return (d.*p)(4); }
int main() {
 D d; d.pad=99; d.x=7; d.z=12;
 int D::* p=raw;
 int (D::* f)(int) const=fn;
 if (data_call(d,p)!=7 || d.*data!=7 || d.*constant_data!=7) return 1;
 if (fn_call(d,f)!=11 || (d.*global_fn)(3)!=10) return 2;
 int B::* n=0; int D::* nd=n;
 int (B::* nf)(int) const=nullptr; int (D::* ndf)(int) const=nf;
 if (nd || ndf || nd!=nullptr || ndf!=nullptr) return 3;
 if (!p || !f || p!=data || f!=global_fn) return 4;
 int B::* back=static_cast<int B::*>(p);
 int (B::* backfn)(int) const=static_cast<int (B::*)(int) const>(f);
 if (d.*back!=7 || (d.*backfn)(2)!=9) return 5;
 const int D::* cp=p; if (d.*cp!=7) return 6;
 return 0;
}
