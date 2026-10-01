template<class T> const void* erased(const T* p){return p;}
template<class T> int temporary(){const _Atomic(T)& x={3};return x;}
int main(){
 _Atomic(int) x=7;
 void* p=&x;const void* cp=&x;volatile void* vp=&x;
 if(p!=cp || cp!=vp || erased(&x)!=cp)return 1;
 const _Atomic(int) cx=9;const void* q=&cx;
 if(static_cast<const _Atomic(int)*>(q)!=&cx)return 2;
 int* plain=reinterpret_cast<int*>(&x);
 if(reinterpret_cast<_Atomic(int)*>(plain)!=&x)return 3;
 int& raw=reinterpret_cast<int&>(x);raw=11;if(x!=11)return 4;
 const _Atomic(int)& r=3;
 return r==3 && temporary<int>()==3?0:5;
}
