typedef int (^B)(int);struct C{operator B();};int f(C c){return c(1);}
