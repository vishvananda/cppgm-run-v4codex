struct Value { int read() && { return 1; } };
int main() { Value v={}; int (Value::*p)() &&=&Value::read; return (v.*p)(); }
