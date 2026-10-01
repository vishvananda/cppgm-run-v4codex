static_assert(sizeof(char[4294967296ULL])==4294967296ULL,"wide semantic layout survives without object emission");
int main(){return 0;}
