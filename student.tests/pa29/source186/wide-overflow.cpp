int main(){unsigned _BitInt(93) x=0;volatile unsigned _BitInt(93) a=-1,b=1;bool over=__builtin_add_overflow(a,b,&x);return !over||x!=0;}
