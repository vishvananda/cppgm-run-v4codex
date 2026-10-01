using F=_Complex float;
using D=_Complex double;
using L=_Complex long double;
extern "C" {
void native_f(F); void native_d(D); void native_l(L);
void host_f(F); void host_d(D); void host_l(L);
int native_catches();
const void* native_type();
const void* native_pointer_type();
}
