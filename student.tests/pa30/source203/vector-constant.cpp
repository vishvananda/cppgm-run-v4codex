typedef int V __attribute__((vector_size(16)));
constexpr V values={1,3,5,7};
static_assert(values[2]==5,"constant lane");
template<int I> struct Read { static constexpr int value=values[I]; };
static_assert(Read<3>::value==7,"substituted constant lane");
int main(){return values[1]!=3;}
