struct A{int x:3;}; int main(){A a={}; (void)const_cast<int&>(a.x);}
