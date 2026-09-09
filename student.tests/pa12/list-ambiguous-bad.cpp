struct X { X(int); X(double); };
struct Y { Y(char const*); };
void use(X);
void use(Y);
int main() { use({0}); }
