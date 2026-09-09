int initialized;
struct Item { int value; Item() : value(++initialized) {} };
thread_local Item tls;
int __cppgm_tls_guard_tls = 7;
void __cppgm_tls_init_tls() {}
int main() {
    if (initialized != 0) return 1;
    if (tls.value != 1 || initialized != 1) return 2;
    return tls.value == 1 && initialized == 1 && __cppgm_tls_guard_tls == 7 ? 0 : 3;
}
