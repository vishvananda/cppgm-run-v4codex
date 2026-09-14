int source = 9;
int read() { static const int &reference = source; return reference; }
