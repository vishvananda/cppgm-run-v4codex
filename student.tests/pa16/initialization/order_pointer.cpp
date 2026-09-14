// N3485: 3.6.2 [basic.start.init]/2; 5.19 [expr.const].
constexpr const int*address_of(const int&v){return &v;}static const int stored=5;extern const int*const address;const int*read(){return address;}const int*seen=read();constexpr const int*address=address_of(stored);int main(){return seen==&stored?0:1;}
