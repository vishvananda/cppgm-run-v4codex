using I=int __attribute__((ext_vector_type(4)));
template<int N> I fixed(){I x{N,2,3,nullptr};return x;}
