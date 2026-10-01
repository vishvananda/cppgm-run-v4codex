template<class T> T outer(T x) { auto f=[]<class U>(U value) { return value; }; return f(x); }
template int outer<int>(int);
