// One selected prototype owns each demanded out-of-class definition.
template<class T> struct Choices {
    int read(int) const;
    int read(double) const;
    int read(long) const;
};
template<class U> int Choices<U>::read(int x) const { return x+sizeof(U); }
template<class U> int Choices<U>::read(double x) const { return int(x)+2*sizeof(U); }
template<class U> int Choices<U>::read(long) const { return U::missing; }
int main() {
    Choices<int> a; Choices<long> b;
    return a.read(3)!=7 || a.read(4)!=8 || b.read(2.0)!=18 || b.read(3.0)!=19;
}
