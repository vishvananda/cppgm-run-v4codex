template<class T, unsigned Bytes> struct Shape {
 typedef T V __attribute__((vector_size(Bytes)));
 static_assert(sizeof(V)==Bytes,"dependent width");
 static constexpr unsigned width=sizeof(V);
};
template<unsigned N> using IntVector __attribute__((vector_size(N))) = int;
static_assert(sizeof(IntVector<16>)==16,"alias width");
static_assert(sizeof(Shape<float,32>::V)==32,"dependent lane and width");
static_assert(!__is_same(Shape<int,16>::V, Shape<int,32>::V),"width identity");
template<class T> unsigned extent(){return sizeof(typename Shape<T,16>::V);}
int main(){return extent<int>()==16 && extent<float>()==16?0:1;}
