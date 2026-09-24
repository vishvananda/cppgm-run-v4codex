// Nonzero qualified receiver: source -> semantic path -> typed LowIR -> supplied ELF backend.
struct Pad{long n;};struct B{int n;int f(){return n;}};template<class T>struct Mid:Pad,T{};template<class T>struct D:Mid<T>{int f(){return Mid<T>::f()+1;}};int main(){D<B>x;x.Pad::n=8;x.B::n=13;return x.f()!=14;}
