typedef int Ints __attribute__((vector_size(8)));
Ints plain;
volatile Ints observed;
Ints explicit_zero{};
Ints rows[3] = {{5,6}};
struct Object { Ints lanes; int tail; } object{};
int local() { static Ints value; return __builtin_ia32_vec_ext_v2si(value,0); }
int main() {
    if (__builtin_ia32_vec_ext_v2si(plain,0) || __builtin_ia32_vec_ext_v2si(plain,1) ||
        __builtin_ia32_vec_ext_v2si(observed,0) || __builtin_ia32_vec_ext_v2si(observed,1) ||
        __builtin_ia32_vec_ext_v2si(explicit_zero,1) || local()) return 1;
    if (__builtin_ia32_vec_ext_v2si(rows[0],1)!=6 || __builtin_ia32_vec_ext_v2si(rows[2],1) ||
        __builtin_ia32_vec_ext_v2si(object.lanes,1) || object.tail) return 2;
    observed=__builtin_ia32_vec_init_v2si(17,29);
    return __builtin_ia32_vec_ext_v2si(observed,1)!=29;
}
