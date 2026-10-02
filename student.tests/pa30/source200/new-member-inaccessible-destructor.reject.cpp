struct Hidden { int value; private: ~Hidden() {} };
struct Aggregate { Hidden member; };
void f() { new Aggregate{{7}}; }
