template<class T> using V __attribute__((ext_vector_type(2)))=T;
static_assert(__is_same(decltype(V<float>{}==V<float>{}),V<int>),"float mask");
static_assert(__is_same(decltype(V<double>{}==V<double>{}),V<long>),"double mask");
static_assert(__is_same(decltype(V<long long>{}==V<long long>{}),V<long>),"long long mask");
static_assert(__is_same(decltype(V<unsigned long>{}==V<unsigned long>{}),V<long>),"unsigned long mask");
int main(){return 0;}
