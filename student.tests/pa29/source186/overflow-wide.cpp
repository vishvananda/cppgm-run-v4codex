template<int N> int test() {
 using U=unsigned _BitInt(N); using S=_BitInt(N);
 U umax=static_cast<U>(-1),u=0;S smax=static_cast<S>(umax>>1),smin=-smax-1,s=0;
 if(!__builtin_add_overflow(umax,U(1),&u)||u!=0)return 1;
 if(!__builtin_sub_overflow(U(0),U(1),&u)||u!=umax)return 2;
 if(!__builtin_mul_overflow(umax,U(2),&u)||u!=umax-1)return 3;
 if(!__builtin_add_overflow(smax,S(1),&s)||s!=smin)return 4;
 if(!__builtin_sub_overflow(smin,S(1),&s)||s!=smax)return 5;
 if(!__builtin_mul_overflow(smin,S(-1),&s)||s!=smin)return 6;
 if(__builtin_mul_overflow(S(-7),U(9),&s)||s!=-63)return 7;
 if(__builtin_add_overflow(umax,S(-1),&u)||u!=umax-1)return 8;
 if(!__builtin_sub_overflow(U(0),S(1),&u)||u!=umax)return 9;
 volatile U vu=0;
 if(!__builtin_add_overflow(umax,U(1),&vu)||vu!=0)return 10;
 return 0;
}
int main(){return test<7>()||test<65>()||test<93>()||test<127>()||test<128>();}
