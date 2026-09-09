struct Value { int read() { return 1; } };
int main() { const Value value={}; int (Value::*read)()=&Value::read; return (value.*read)(); }
