int loops(int mode) { for(;;) { switch(mode) { case 0:return 2; default:break; } } }
int literal_loop(){while(1){}}
int true_loop(){do {}while(true);}
int goto_loop(){ again: goto again; }
int switch_return(int x){ switch(x){case 0:return 3;default:return 4;} }
int label_return(){goto L; if(false){L:return 5;} }
int throw_only(){throw 6;}
int guarded(int x){try {if(x)throw 6;return 7;}catch(int){return 8;}}
int main(){return loops(0)+switch_return(0)+label_return()+guarded(1)-18;}
