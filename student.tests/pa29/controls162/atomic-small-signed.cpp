int main(){signed char x=-3;return __atomic_fetch_add(&x,1,5)==-3 && x==-2?0:1;}
