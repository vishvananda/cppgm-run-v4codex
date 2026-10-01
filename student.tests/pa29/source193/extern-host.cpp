#include "extern.h"
template struct Box<int>::Inner;
extern int call(int);
int main(){return call(3)!=10 || Box<int>::Inner::value(2)!=9;}
