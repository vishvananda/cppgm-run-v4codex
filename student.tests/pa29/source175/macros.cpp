#include "macro-header.h"
extern "C" int printf(const char*,...);
#line 701 "call-site.cpp"
static_assert(OUTER_LINE==701,"macro expansion");
static_assert(CALL_SITE==702,"macro default invocation");
static_assert(FILE_SITE[0]=='c',"macro file invocation");
int main(){printf("%d %d %s\n",OUTER_LINE,CALL_SITE,FILE_SITE);}
