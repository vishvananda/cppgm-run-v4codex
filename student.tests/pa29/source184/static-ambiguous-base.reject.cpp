struct A{}; struct B:A{};struct C:A{};struct D:B,C{}; int main(){D d; (void)(A*)&d;}
