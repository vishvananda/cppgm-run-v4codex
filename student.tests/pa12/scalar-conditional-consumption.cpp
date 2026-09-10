int trace;
struct Pair{long first,second;Pair(long n):first(n),second(n+1){}};
Pair make(long n){return Pair(n);}
struct Watch{
 long* out;long expected;
 Watch(long* p,long n):out(p),expected(n){trace=trace*10+1;}
 ~Watch(){if(out&&*out!=expected)__builtin_abort();trace=trace*10+3;}
};
long read(Pair const& p,Watch const&){trace=trace*10+2;return p.first+p.second;}
long dynamic(bool choose){
 trace=0;
 long result=(choose?read(make(4),Watch(&result,9)):read(make(7),Watch(&result,15)));
 if(trace!=123)__builtin_abort();return result;
}
long constant_true(){trace=0;bool choose=true;
 long result=choose?read(make(4),Watch(0,0)):read(make(7),Watch(0,0));
 if(trace!=123)__builtin_abort();return result;}
long constant_false(){trace=0;bool choose=(false);
 long result=choose?read(make(4),Watch(0,0)):read(make(7),Watch(0,0));
 if(trace!=123)__builtin_abort();return result;}
long volatile_condition(){trace=0;volatile bool choose=true;choose=false;
 long result=choose?read(make(4),Watch(0,0)):read(make(7),Watch(0,0));
 if(trace!=123)__builtin_abort();return result;}
long narrow_condition(){trace=0;unsigned char choose=256;
 long result=choose?read(make(4),Watch(0,0)):read(make(7),Watch(0,0));
 if(trace!=123)__builtin_abort();return result;}
long unevaluated_condition(){trace=0;bool choose=false;(void)sizeof(++choose);
 long result=choose?read(make(4),Watch(0,0)):read(make(7),Watch(0,0));
 if(trace!=123)__builtin_abort();return result;}
void flip(bool& value){value=false;}
long mutable_condition(int mode){trace=0;bool choose=true;
 if(mode==0)choose=false;
 if(mode==1){bool& ref=choose;ref=false;}
 if(mode==2)flip(choose);
 if(mode==3){bool other=true;(mode==3?choose:other)=false;}
 long result=choose?read(make(4),Watch(0,0)):read(make(7),Watch(0,0));
 if(trace!=123)__builtin_abort();return result;}
struct RefWatch{long& out;long expected;
 RefWatch(long& p,long n):out(p),expected(n){}
 ~RefWatch(){if(out!=expected)__builtin_abort();}};
long read_ref(Pair const& p,RefWatch const&){return p.first+p.second;}
long reference_destination(bool choose){
 long result=choose?read_ref(make(4),RefWatch(result,9)):read_ref(make(7),RefWatch(result,15));return result;}
struct NarrowWatch{short* out;short expected;
 NarrowWatch(short* p,short n):out(p),expected(n){}
 ~NarrowWatch(){if(*out!=expected)__builtin_abort();}};
long read_narrow(Pair const& p,NarrowWatch const&){return p.first+p.second;}
short narrow_destination(bool choose){
 short result=choose?read_narrow(make(4),NarrowWatch(&result,9)):read_narrow(make(7),NarrowWatch(&result,15));return result;}
long known_observed_destination(){trace=0;bool choose=true;
 long result=choose?read(make(4),Watch(&result,9)):read(make(7),Watch(&result,15));
 if(trace!=123)__builtin_abort();return result;}
short known_narrow_destination(){bool choose=false;
 short result=choose?read_narrow(make(4),NarrowWatch(&result,9)):read_narrow(make(7),NarrowWatch(&result,15));return result;}
int main(){
 if(dynamic(true)!=9||dynamic(false)!=15)return 1;
 if(constant_true()!=9||constant_false()!=15)return 2;
 for(int i=0;i<4;++i)if(mutable_condition(i)!=15)return 3;
 if(reference_destination(true)!=9||reference_destination(false)!=15)return 4;
 if(narrow_destination(true)!=9||narrow_destination(false)!=15)return 5;
 if(volatile_condition()!=15||narrow_condition()!=15||unevaluated_condition()!=15)return 6;
 if(known_observed_destination()!=9||known_narrow_destination()!=15)return 7;
 return 0;
}
