[[noreturn]] void standard_stop(){throw 1;}
void gnu_stop() __attribute__((__noreturn__)); void gnu_stop(){throw 2;}
template<class T> [[noreturn]] void stop(T x){throw x;}
struct X { [[noreturn]] void stop(){throw 4;} };
int a(){standard_stop();} int b(){if(false)return 0;gnu_stop();}
int c(){stop(3);} int d(){X().stop();}
int main(){int total=0;try{a();}catch(int x){total+=x;}try{b();}catch(int x){total+=x;}try{c();}catch(int x){total+=x;}try{d();}catch(int x){total+=x;}return total-10;}
