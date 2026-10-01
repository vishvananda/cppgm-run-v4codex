struct S{unsigned n;S()=default;S(const S&)=delete;};
unsigned f(const S& s){return __builtin_bit_cast(unsigned,s);}
int main(){S s;s.n=17;return f(s)==17?0:1;}
