int conversions;
struct Count{unsigned n;operator unsigned()const{++conversions;return n;}};
struct Signed{short n;operator short()const{return n;}};
struct Reference{int n;operator int&(){++conversions;return n;}};
struct Derived:Count{};
template<class T,class N>auto allocate(N n)->decltype(new T[n]){return new T[n];}
int main(){int*p=allocate<int>(Count{4});p[3]=17;int*q=new int[Signed{3}];q[2]=19;
Reference r{2};int*s=new int[r];s[1]=23;Derived d;d.n=2;int*t=new int[d];t[1]=29;
int result=p[3]+q[2]+s[1]+t[1]-88;delete[]p;delete[]q;delete[]s;delete[]t;return result || conversions!=3;}
