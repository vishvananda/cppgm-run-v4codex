int destroyed;
struct G{~G(){++destroyed;}};
struct D{G g;~D()noexcept(false)try{throw 7;}catch(int){return;}};
int main(){try{D d;}catch(...){return 2;}return destroyed!=1;}
