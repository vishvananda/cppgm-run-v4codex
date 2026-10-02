int no(){__builtin_unreachable();} int die(){__builtin_abort();} int f(){if(true)return 3;} int main(){return f()-3;}
