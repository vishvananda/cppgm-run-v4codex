template<class T> struct Lanes { typedef T V __attribute__((vector_size(16))); V value; T read(int i) const { return value[i]; } void write(int i,T v){ value[i]=v; } };
int main(){Lanes<short> x={};x.write(5,31);return x.read(5)!=31;}
