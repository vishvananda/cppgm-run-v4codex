#include "layout.h"
extern "C" int verify(HostS* s,HostT* t,HostU* u){return s->c!='s'||s->x!=7||t->c!='t'||t->x!=8||u->c!='u'||(char*)&u->e!=(char*)u;}
int main(){HostS s={'s',7,{}};HostT t={'t',8};HostU u={'u',{}, {}};return update(&s,&t,&u)||s.x!=8||t.x!=10||owned();}
