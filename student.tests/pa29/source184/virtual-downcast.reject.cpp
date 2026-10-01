struct A{};struct B:virtual A{}; int main(){A*p=0; (void)(B*)p;}
