#include "virtual-primary153.h"
VRoot::VRoot(){observe153(1,this,value());}
VRoot::~VRoot(){observe153(2,this,value());}
long VRoot::value(){return 10;}
VRoot* VRoot::address(){return this;}
VOwner::VOwner(){observe153(3,this,value());}
VOwner::~VOwner(){observe153(4,this,value());}
VRoot* VOwner::self(){return this;}
VRoot& VOwner::ref(){return *this;}
VDerived::VDerived(){words[0]=37;observe153(5,this,value());}
VDerived::~VDerived(){observe153(6,this,value());}
VDerived* VDerived::self(){return words[0] ? this : 0;}
VDerived& VDerived::ref(){return *this;}
long VDerived::value(){return words[0];}
VMore::VMore(){extra[0]=51;observe153(7,this,value());}
VMore::~VMore(){observe153(8,this,value());}
Left153::Left153():left(61){observe153(9,this,value());}
Left153::~Left153(){observe153(10,this,value());}
long Left153::value(){return left;}
Right153::Right153():right(71){observe153(11,this,value());}
Right153::~Right153(){observe153(12,this,value());}
long Right153::value(){return right;}
Diamond153::Diamond153():data(81){observe153(13,this,value());}
Diamond153::~Diamond153(){observe153(14,this,value());}
long Diamond153::value(){return data+left+right;}
long producer_size(int n){return n ? sizeof(Diamond153) : sizeof(VMore);}
void* create153(int n){if(n)return new Diamond153;return new VMore;}
void destroy153(int n,void* p){if(n)delete static_cast<Diamond153*>(p);else delete static_cast<VMore*>(p);}
Nested153::Nested153(){observe153(15,this,value());}
Nested153::~Nested153(){observe153(16,this,value());}
long Nested153::value(){return 101;}
Nonempty153::Nonempty153():member(111){observe153(17,this,value());}
Nonempty153::~Nonempty153(){observe153(18,this,value());}
long Nonempty153::value(){return member;}
Indirect153::Indirect153(){observe153(19,this,value());}
Indirect153::~Indirect153(){observe153(20,this,value());}
long Indirect153::value(){return member+1;}
Another153::~Another153(){}
long Another153::other(){return 122;}
Mixed153::Mixed153():member(131){observe153(21,this,value());}
Mixed153::~Mixed153(){observe153(22,this,value());}
long Mixed153::value(){return member;}
long Mixed153::other(){return member+1;}
Multi153::Multi153():member(141){observe153(23,this,value());}
Multi153::~Multi153(){observe153(24,this,value());}
long Multi153::value(){return member;}
long Multi153::other(){return member+1;}
VirtualMulti153::VirtualMulti153():extra(151){observe153(25,this,value());}
VirtualMulti153::~VirtualMulti153(){observe153(26,this,value());}
long VirtualMulti153::value(){return member+extra;}
long VirtualMulti153::other(){return member+extra+1;}
long InlineKey153::key(){return 161;}
Choice153::Choice153(){observe153(27,this,value());}
Choice153::~Choice153(){observe153(28,this,value());}
long Choice153::value(){return member+61;}
Aligned153::~Aligned153(){}
