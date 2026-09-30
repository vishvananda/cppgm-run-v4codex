struct Pair { long a; long b; };
Pair incoming[3] = {{2,4},{5,9},{17,1}};
double imported_fp = 1.5;
int imported_count = 7;
Pair* host_address() { return incoming + 2; }
