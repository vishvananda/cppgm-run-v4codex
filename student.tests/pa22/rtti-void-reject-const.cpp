struct B {virtual int f();}; void*f(const B*b){return dynamic_cast<void*>(b);}
