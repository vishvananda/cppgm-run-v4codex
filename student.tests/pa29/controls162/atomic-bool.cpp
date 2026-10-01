int main(){_Atomic(bool) x=false;x+=0;if(x)return 1;x=true;x&=false;return x?2:0;}
