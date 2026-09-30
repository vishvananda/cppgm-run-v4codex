#include "virtual-primary153.h"
void observe_other153(int n,Another153* p,long expected){printf("other-life %d %ld %ld\n",n,p->other(),expected);}
void observe153(int n,VRoot* root,long expected){
  printf("life %d %ld %ld %d\n",n,root->value(),expected,root->address()==root);
}
void check_more(VMore* p){
  VOwner* owner=p; VRoot* root=p;
  printf("more %ld %ld %ld %ld %d %d %d %d\n",long(sizeof(VMore)),producer_size(0),
    long(reinterpret_cast<char*>(root)-reinterpret_cast<char*>(p)),root->value(),
    owner->self()==root,&owner->ref()==root,dynamic_cast<VMore*>(root)==p,dynamic_cast<void*>(root)==p);
  p->words[0]=0; printf("null %d\n",owner->self()==0); p->words[0]=37;
}
void check_diamond(Diamond153* p){
  Left153* left=p;Right153* right=p;VRoot* root=p;
  printf("diamond %ld %ld %ld %ld %ld %ld %d %d %d\n",long(sizeof(Diamond153)),producer_size(1),
    long(reinterpret_cast<char*>(root)-reinterpret_cast<char*>(p)),
    long(reinterpret_cast<char*>(right)-reinterpret_cast<char*>(p)),left->value(),right->value(),
    dynamic_cast<Right153*>(root)==right,dynamic_cast<Left153*>(right)==left,dynamic_cast<void*>(right)==p);
}
int main(){
  { MoreMix153 p;VRoot* root=&p;Another153* other=&p;
    printf("nonvirtualmix %ld %ld %ld %d\n",long(sizeof(p)),root->value(),other->other(),dynamic_cast<Another153*>(root)==other); }

  { AlignOwner153 p;Aligned153* base=&p;
    printf("aligned %ld %ld %ld\n",long(sizeof(p)),long(alignof(AlignOwner153)),long(reinterpret_cast<char*>(base)-reinterpret_cast<char*>(&p))); }

  { Choice153 p;VRoot* root=&p;Nonempty153* first=&p;VOwner* primary=&p;
    printf("choice %ld %ld %ld %ld %ld %d %d\n",long(sizeof(p)),long(reinterpret_cast<char*>(root)-reinterpret_cast<char*>(&p)),long(reinterpret_cast<char*>(first)-reinterpret_cast<char*>(&p)),long(reinterpret_cast<char*>(primary)-reinterpret_cast<char*>(&p)),root->value(),root->address()==root,dynamic_cast<VOwner*>(first)==primary); }

  { InlineKey153 p;VRoot* root=&p;Another153* other=&p;
    printf("inline-key %ld %ld %ld %ld %ld %ld %d\n",long(sizeof(p)),long(reinterpret_cast<char*>(root)-reinterpret_cast<char*>(&p)),long(reinterpret_cast<char*>(other)-reinterpret_cast<char*>(&p)),root->value(),other->other(),p.key(),dynamic_cast<Another153*>(root)==other); }

  { VirtualMulti153 p;VRoot* root=&p;Another153* other=&p;
    printf("virtualmulti %ld %ld %ld %ld %ld %d %d\n",long(sizeof(p)),long(reinterpret_cast<char*>(root)-reinterpret_cast<char*>(&p)),long(reinterpret_cast<char*>(other)-reinterpret_cast<char*>(&p)),root->value(),other->other(),root->address()==root,dynamic_cast<Another153*>(root)==other); }

  { Nested153 p;VOwner* owner=&p;VRoot* root=&p;
    printf("nested %ld %ld %ld %d %d\n",long(sizeof(p)),long(reinterpret_cast<char*>(root)-reinterpret_cast<char*>(&p)),root->value(),owner->self()==root,dynamic_cast<Nested153*>(root)==&p); }
  { Indirect153 p;Nonempty153* base=&p;VRoot* root=&p;
    printf("indirect %ld %ld %ld %ld %ld %d\n",long(sizeof(p)),long(reinterpret_cast<char*>(base)-reinterpret_cast<char*>(&p)),long(reinterpret_cast<char*>(root)-reinterpret_cast<char*>(&p)),root->value(),base->value(),dynamic_cast<Indirect153*>(root)==&p); }
  { Mixed153 p;VOwner* owner=&p;VRoot* root=&p;Another153* other=&p;
    printf("mixed %ld %ld %ld %ld %ld %d %d\n",long(sizeof(p)),long(reinterpret_cast<char*>(root)-reinterpret_cast<char*>(&p)),long(reinterpret_cast<char*>(other)-reinterpret_cast<char*>(&p)),root->value(),other->other(),owner->self()==root,dynamic_cast<Another153*>(root)==other); }

  {VMore object;check_more(&object);}
  void* p=create153(0);check_more(static_cast<VMore*>(p));destroy153(0,p);
  {Diamond153 object;check_diamond(&object);}
  p=create153(1);check_diamond(static_cast<Diamond153*>(p));destroy153(1,p);
}
