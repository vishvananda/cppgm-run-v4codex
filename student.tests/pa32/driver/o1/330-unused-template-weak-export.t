// Checking an unused inline body does not demand its non-inline template.
template<class T> __attribute__((noinline)) T helper(T value) { return value + 1; }
inline int unused_template_call() { return helper(6); }
// This ordinary weak definition is independently available to another TU.
extern "C" __attribute__((weak,noinline)) int exported_weak() { return 17; }
int main() { return 0; }
