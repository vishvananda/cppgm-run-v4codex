struct S{int f(const int& a){return a;}};int main(){S s;_Atomic(int) a=7;return __builtin_invoke(&S::f,s,a)==7?0:1;}
