extern int g;
int* address_of_g();
int main() { return address_of_g() == &g && g == 7 ? 0 : 1; }
