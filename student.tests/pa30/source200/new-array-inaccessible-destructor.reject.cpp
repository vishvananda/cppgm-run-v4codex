struct Hidden { int value; private: ~Hidden() {} };
void f() { new Hidden[2]{}; }
