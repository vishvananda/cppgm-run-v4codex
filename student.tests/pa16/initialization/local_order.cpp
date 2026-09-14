// N3485: 3.6.2 [basic.start.init]/2; 5.19 [expr.const].
int read();int seen=read();int source=9;int read(){static const int&ref=source;return ref;}int main(){return seen==9?0:1;}
