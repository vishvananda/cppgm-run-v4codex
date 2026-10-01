struct Base {int a;};struct Derived:Base {int b;};
_Atomic(Derived) object;
Base* p=&object;
