#include "layout.h"
extern "C" int update(HostS* s,HostT* t,HostU* u){
 if((char*)&s->e!=(char*)s || (char*)&u->e!=(char*)u || (char*)&u->h==(char*)&u->e) return 1;
 ++s->x; t->x+=2;return 0;
}
extern "C" int owned(){HostS s={'s',7,{}};HostT t={'t',8};HostU u={'u',{}, {}};return verify(&s,&t,&u);}
