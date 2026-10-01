int live=0, destroyed=0;
struct Tracked {
 int value;
 explicit Tracked(int v):value(v){++live;}
 Tracked(const Tracked&)=delete;
 ~Tracked(){--live;++destroyed;}
};
int consume(Tracked&& x){return live==1?x.value:0;}
int main(){
 int consumed=consume(const_cast<Tracked&&>(Tracked(7)));
 if(consumed!=7 || live!=0 || destroyed!=1)return 1;
 {Tracked&& ref=const_cast<Tracked&&>(Tracked(11));
  if(live!=1 || ref.value!=11)return 2;
 }
 if(live!=0 || destroyed!=2)return 3;
 try {Tracked&& ref=const_cast<Tracked&&>(Tracked(13));
  if(live!=1 || ref.value!=13)return 4;
  throw 19;
 }catch(int value){if(value!=19 || live!=0 || destroyed!=3)return 5;}
 return 0;
}
