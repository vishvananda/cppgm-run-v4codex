struct X { X(int) = delete; X(double); };
void use(X);
int main() { use({2}); }
