typedef int V __attribute__((vector_size(8)));
volatile V value={7,9};
int main(){unsigned long long bits=__builtin_bit_cast(unsigned long long,value);return (unsigned)bits!=7||bits>>32!=9;}
