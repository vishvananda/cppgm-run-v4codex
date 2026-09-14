// N3485 [basic.start.init]/2 and [stmt.dcl]/4, across translation units.
int read();
int seen = read();
int main() { return seen == 9 ? 0 : 1; }
