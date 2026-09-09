struct Value { int& n; };
int main() { int Value::*p=&Value::n; }
