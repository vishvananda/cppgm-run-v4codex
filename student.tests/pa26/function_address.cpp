extern "C" int imported(int);
extern "C" int (*host_address())(int);
extern "C" int call_pointer(int (*)(int), int);
static int local(int n) { return n+3; }
int main(int argc,char**) {
    int (*p)(int)=argc==1 ? imported : local;
    return p==host_address() && p(9)==20 && call_pointer(local,4)==7 ? 0:1;
}
