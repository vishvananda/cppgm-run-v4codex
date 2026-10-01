int main(){_Atomic(bool) x=false;if(x++ || !x)return 1;if(!x++ || !x)return 2;return ++x?0:3;}
