typedef int Ints __attribute__((vector_size(8)));
volatile Ints observed = {17,29};
int read_lane() { return __builtin_ia32_vec_ext_v2si(observed,1); }
unsigned long long read_bits() { return (unsigned long long)observed; }
int main() { return read_lane()!=29 || read_bits()!=((29ull<<32)|17); }
