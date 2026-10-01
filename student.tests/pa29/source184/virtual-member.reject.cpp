struct A{int x;};struct B:virtual A{}; int main(){(void)(int B::*)&A::x;}
