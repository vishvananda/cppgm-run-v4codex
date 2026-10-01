template<class T> int f(T b){return b("bad");} int g(int (^b)(int)){return f(b);}
