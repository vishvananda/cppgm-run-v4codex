int g = 7;
int read_g() { return ++g; }
int main() { return read_g() == 8 && g == 8 ? 0 : 1; }
