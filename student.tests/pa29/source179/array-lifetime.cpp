int live, copies, attempts, fail_at;
struct Item {int value;Item(int v=3):value(v){++live;}Item(const Item& x):value(x.value){if(++attempts==fail_at)throw attempts;++live;++copies;}~Item(){--live;}};
int small(){Item source[3];try{auto [a,b,c]=source;if(live!=6||copies!=3||a.value+b.value+c.value!=9)return 1;}catch(...){return 2;}return live!=3;}
int large(){Item source[10];fail_at=attempts+7;try{auto [a,b,c,d,e,f,g,h,i,j]=source;return 1;}catch(int x){if(x!=fail_at||live!=10)return 2;}fail_at=0;{auto [a,b,c,d,e,f,g,h,i,j]=source;if(live!=20||j.value!=3)return 3;}return live!=10;}
int main(){if(small()||live)return 1;if(large()||live)return 2;return 0;}
