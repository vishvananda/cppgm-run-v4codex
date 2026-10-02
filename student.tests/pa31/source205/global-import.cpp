extern int g;
int read_g() { return ++g; }
int main() { return read_g() == 8 && g == 8 ? 0 : 1; }
