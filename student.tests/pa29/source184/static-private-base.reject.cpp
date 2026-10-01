struct A{};struct B:private A{}; int main(){B*p=0; (void)static_cast<A*>(p);}
