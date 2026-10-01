extern inline void unsupported(){ __builtin_unknown_instruction_167(); }
inline void dormant(){ unsupported(); }
inline int used_leaf(int x){return x+4;}
inline int used_branch(int x){return used_leaf(x)*3;}
inline int address_only(int x){return x-2;}
inline int unused_scalar(){return 997;}
int (*selected)(int)=address_only;
int main(int argc,char**){return used_branch(argc)==15 && selected(argc)==-1?0:1;}
