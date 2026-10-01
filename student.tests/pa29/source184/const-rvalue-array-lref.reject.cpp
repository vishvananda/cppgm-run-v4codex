int main(){int a[1]={}; (void)const_cast<int(&)[1]>(static_cast<int(&&)[1]>(a));}
