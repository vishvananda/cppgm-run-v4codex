// N3485: 3.6.2 [basic.start.init]/2; 5.19 [expr.const].
int object;extern int&ref;int*read(){return &ref;}int*seen=read();constexpr int&ref=object;int main(){return seen==&object?0:1;}
