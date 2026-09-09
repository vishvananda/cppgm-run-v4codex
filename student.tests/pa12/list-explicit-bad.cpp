struct X { explicit X(int); };
void use(X);
int main() { use({2}); }
