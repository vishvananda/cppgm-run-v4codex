// Runtime vector values are outside PA29; do not silently lower as scalar.
typedef int V __attribute__((vector_size(16)));
V demanded(){return V{1,2,3,4};}
int main(){demanded();}
