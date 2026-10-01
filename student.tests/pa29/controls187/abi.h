using F = _Complex float;
using D = _Complex double;
using L = _Complex long double;
extern "C" {
F f_echo(F);
D d_many(D,D,D,D,D,D,int,double);
L l_echo(L);
D host_step(D);
L host_wide(L);
D call_host(D);
L call_wide(L);
int ignore_wide();
}
