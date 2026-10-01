using V=int __attribute__((vector_size(32)));
extern "C" { V native_vector(V); V host_vector(V); int native_check(); }
