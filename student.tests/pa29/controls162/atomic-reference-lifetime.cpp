int main(){_Atomic(int) x=3;const int& r=x;x=9;return r==3?0:1;}
