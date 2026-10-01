inline int forward(int);
inline int twice(int x){return forward(x)+forward(x);}
inline int forward(int x){return x+5;}
inline int recursive(int x){return x?recursive(x-1)+1:0;}
inline int dormant_recursion(int x){return x?dormant_recursion(x-1)+1:0;}
inline int unused_unknown(){return 1;}
int main(int argc,char**){return twice(argc)==12 && recursive(7)==7?0:1;}
