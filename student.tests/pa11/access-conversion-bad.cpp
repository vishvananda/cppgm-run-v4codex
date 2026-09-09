struct Base {};
class Derived : Base {};
void use(const Base*);
int main() { Derived value; use(&value); }
